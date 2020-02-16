#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <mem.h>

#include "memory\DMA.h"
#include "memory\PMem.h"
#include "Time.h"

#define SBLASTER_IMPORT
#include "audio\SBlaster.h"

enum DMAMode {
    DMAMode_SINGLE_8BIT, DMAMode_AUTO_8BIT
};

/* DSP flags */
#define SPEAKER_ON          0xD1
#define SPEAKER_OFF         0xD3

#define SET_DAC             0x10
#define READ_DAC            0x20

#define DMA_MODE_SET        0x0B
#define Timer_CONSTANT_PORT 0x40
#define SINGLE_8_BIT        0x14
#define DMA_STOP            0xD3
#define BLOCK_XFER_SIZE     0x48
#define EXIT_8b_AUTO_INIT   0xDA
#define outportbUT_8b_AUTO_INIT 0x1C

/* Interrupt flags */
#define INTERRUPT_PORT      0x21
#define EOI                 0x20
#define PIC1_COMMAND        0x20
#define PIC2_COMMAND        0xA0
#define CLEAR_FLIP_FLOP     0x0C
#define MASK_CHANNEL        0x0A
#define DMA_AUTO_INIT_8_BIT 0x58
#define DMA_SINGLE_8_BIT    0x48

/* DMA Buffer */
#define MAX_BUFFER          0x200
static unsigned long bufferSize;
static unsigned long halfBufferSize;
static unsigned char* dmaBuffer;
static volatile unsigned int accumulator;
static unsigned int accumulatorMax;
static unsigned char* dataStart;
static volatile unsigned char toggle;

static const unsigned int PagePorts[] = { 0x87, 0x83, 0x81, 0x82 };
static const unsigned int AddressPorts[] = { 0x00, 0x02, 0x04, 0x06 };
static const unsigned int LengthPorts[] = { 0x01, 0x03, 0x05, 0x07 };
static const unsigned int ModeRegisterPorts[] = { 0x48, 0x49, 0x4A, 0x4B };
static unsigned int baseAddr;
static unsigned int mixerAddr;
static unsigned int IRQ;
static unsigned int MPU_AD;
static unsigned int DMA_8_CHAN;
static unsigned int DMA_16_CHAN;

static unsigned int cardVersion;
static unsigned int ISRSet = 0;
static unsigned int dmaOffset;

#define DSP_RESET_AD        0x6+baseAddr
#define MIXER_AD            0x4+mixerAddr
#define MIXER_DATA_AD       0x5+mixerAddr
#define DSP_READ_AD         0xA+baseAddr
#define DSP_READ_STATUS_AD  0xE+baseAddr
#define DSP_WRITE_AD        0xC+baseAddr
#define DSP_16_INTER_AD     0xF+baseAddr

#define MPU_STATUS_AD       0x01+MPU_AD
#define MPU_DATA_AD         0x00+MPU_AD

void interrupt (*oldHandler)();

static unsigned int getPort(const char* raw) {
    unsigned int value = 0;
    sscanf(raw, "%u", &value);
    return value;
}

static unsigned int getAddress(const char* raw) {
    unsigned int value = 0;
    sscanf(raw, "%x", &value);
    return value;
}

static char const* bufferRaw(char const *environment, char* const subbuf, int *i, int len) {
    int l = 0;
    while(environment[++*i] != ' ' && *i < len) {
        subbuf[l++] = environment[*i];
    }
    subbuf[l] = '\0';

    return subbuf;
}

static ErrorState parseEnvironment() {
    char const* blasterEnv = getenv("BLASTER");
    int length = strlen(blasterEnv);
    char subbuf[4];
    int i;

    if(!blasterEnv) {
        return ERR_FAIL;
    }

    for(i = 0; i<length; ++i) {
        switch(blasterEnv[i]) {
            case 'A':
                baseAddr = getAddress(bufferRaw(blasterEnv, subbuf, &i, length));
                break;
            case 'I':
                IRQ = getPort(bufferRaw(blasterEnv, subbuf, &i, length));
                break;
            case 'D':
                DMA_8_CHAN = getPort(bufferRaw(blasterEnv, subbuf, &i, length));
                break;
            case 'H':
                DMA_16_CHAN = getPort(bufferRaw(blasterEnv, subbuf, &i, length));
                break;
            case 'T':
                cardVersion = getPort(bufferRaw(blasterEnv, subbuf, &i, length));
                break;
            case 'M':
                mixerAddr = getAddress(bufferRaw(blasterEnv, subbuf, &i, length));
                break;
            case 'P':
                MPU_AD = getAddress(bufferRaw(blasterEnv, subbuf, &i, length));
                break;
        }
    }

    if(!baseAddr || !IRQ) {
        return ERR_FAIL;
    }

    if(!cardVersion && DMA_16_CHAN) {
        cardVersion = 6;
    }

    if(cardVersion >= 6) {
        if(!mixerAddr) {
            mixerAddr = baseAddr;
        }
        if(!MPU_AD) {
            MPU_AD = 0x330;
        }
    }

    if(IRQ < 8) {
        dmaOffset = 0x08;
    }
    else {
        dmaOffset = 0x70;
    }

    return ERR_SUCCESS;
}

unsigned char readDSP() {
    int i;
    unsigned char flag, result;
    for(i=0; i<100; ++i) {
        flag = inportb(DSP_WRITE_AD);
        if(0x80 & flag) {
            break;
        }
    }
    return (unsigned char)inportb(DSP_READ_AD);
}

void writeDSP(unsigned char value) {
    int i;
    unsigned char flag;
    for(i=0; i<100; ++i) {
        flag = inportb(DSP_WRITE_AD);
        if(!(0x80 & flag)) {
            break;
        }
    }
    outportb(DSP_WRITE_AD, (int)value);
}

void speakerSet(unsigned char speakerOn) {
    if(speakerOn) {
        writeDSP(SPEAKER_ON);
    }
    else {
        writeDSP(SPEAKER_OFF);
    }
}

void clearDMABytePointer() {
    outportb(CLEAR_FLIP_FLOP, 0);
    /*_asm {
        mov al,0;
        out CLEAR_FLIP_FLOP,al;
    }*/
}

void maskDMAChannel(unsigned char channel, unsigned char set) {
    outportb(MASK_CHANNEL, channel+ (set ? 4 : 0));
}

void setDMA(unsigned int mode, unsigned int bufferLength) {
    unsigned char addressLow, addressHigh, lengthLow, lengthHigh;
    unsigned int page, currentPort;
    {
        DMAInfo info = DMA_makeDMAInfo(dmaBuffer, bufferLength);
        addressLow = info.addressLow;
        addressHigh = info.addressHigh;
        lengthLow = info.lengthLow;
        lengthHigh = info.lengthHigh;
        page = info.page;
    }
    maskDMAChannel(DMA_8_CHAN, 1);
    clearDMABytePointer();
    if(mode == DMAMode_AUTO_8BIT) {
        outportb(DMA_MODE_SET, DMA_AUTO_INIT_8_BIT+DMA_8_CHAN);
    }
    else if(mode == DMAMode_SINGLE_8BIT) {
        outportb(DMA_MODE_SET, DMA_SINGLE_8_BIT+DMA_8_CHAN);
    }
    /*outportb(AddressPorts[DMA_8_CHAN], info.addressLow);*/
    currentPort = AddressPorts[DMA_8_CHAN];
    _asm {
        mov dx,currentPort;
        mov al,addressLow;
        out dx,al;
        /*outportb(AddressPorts[DMA_8_CHAN], info.addressHigh);*/
        mov al,addressHigh;
        out dx,al;
        /*outportb(PagePorts[DMA_8_CHAN], info.page);*/
        mov bx,page;
        mov al,bl
        out dx,al;
        /*outportb(LengthPorts[DMA_8_CHAN], info.lengthLow);*/
        mov al,lengthLow;
        out dx,al;
        /*outportb(LengthPorts[DMA_8_CHAN], info.lengthHigh);*/
        mov al,lengthHigh;
        out dx,al;
    }
    maskDMAChannel(DMA_8_CHAN, 0);
}

void setTime(WAVFile file) {
    writeDSP(Timer_CONSTANT_PORT);
    writeDSP( 256-(1000000 / file->sampleRate) );
}

unsigned int readMixerAt(unsigned int index) {    
    outportb(MIXER_AD, index);
    return inportb(MIXER_DATA_AD);
}

void writeMixerAt(unsigned int index, unsigned int value) {
    outportb(MIXER_AD, index);
    outportb(MIXER_DATA_AD, value);
}

void setBlockTransferSize() {
    writeDSP(BLOCK_XFER_SIZE);
    writeDSP(LO(halfBufferSize-1));
    writeDSP(HI(halfBufferSize-1));
}

void setBlockSize(unsigned int bufferLength) {
    writeDSP(SINGLE_8_BIT);
    writeDSP(LO(bufferLength-1));
    writeDSP(HI(bufferLength-1));
}

void resetISR() {
    unsigned int interruptVector;
    unsigned char mask;

    if(!ISRSet) {
        return;
    }

    interruptVector = IRQ + dmaOffset;
    setvect(interruptVector, oldHandler);

    mask = inportb(INTERRUPT_PORT);
    outportb(INTERRUPT_PORT, mask | (1<<IRQ));

    ISRSet = 0;
}

void sendEOI() {
    if(IRQ >= 8) {
        outportb(PIC2_COMMAND, 0x20);
    }
    outportb(PIC1_COMMAND, 0x20);
}

void interrupt blasterISR() {
    accumulator += halfBufferSize;

    if(accumulator >= accumulatorMax) {
        writeDSP(EXIT_8b_AUTO_INIT);
        speakerSet(0);
        resetISR();
    }
    else {
        if(!toggle) {
            unsigned int actualIncrease = accumulator + halfBufferSize;
            if(accumulator+halfBufferSize > accumulatorMax) {
                actualIncrease = accumulatorMax;
            }
            memcpy(dmaBuffer, dataStart+actualIncrease, halfBufferSize);
            toggle = 1;
        }
        else {
            unsigned int actualIncrease = accumulator + halfBufferSize;
            if(accumulator+halfBufferSize > accumulatorMax) {
                actualIncrease = accumulatorMax;
            }
            memcpy(dmaBuffer+halfBufferSize, dataStart+actualIncrease, halfBufferSize);
            toggle = 0;
        }

        if(accumulator + halfBufferSize > accumulatorMax) {
            /* if not looping */
            writeDSP(SINGLE_8_BIT);
            writeDSP(LO(halfBufferSize-(accumulatorMax-accumulator)));
            writeDSP(HI(halfBufferSize-(accumulatorMax-accumulator)));
            /* if looping */
        }
    }

    /* Acknowledge ISR */
    /* if 8 bit */
    inportb(DSP_READ_STATUS_AD);

    sendEOI();
}

void setISR() {
    unsigned int interruptVector;
    unsigned int mask;

    if(ISRSet) {
        return;
    }

    interruptVector = IRQ + dmaOffset;

    oldHandler = getvect(interruptVector);
    setvect(interruptVector, &blasterISR);

    mask = inportb(INTERRUPT_PORT);
    outportb(INTERRUPT_PORT, mask & ~(1<<IRQ));

    ISRSet = 1;
}

void resetDSP() {
    /* 15h, ah=86h == delay by CX:DX microseconds */

    /*union REGS regs;
    outportb(DSP_RESET_AD, 1);
    regs.x.cx = 0;
    regs.x.dx = 3;
    regs.h.ah= 0x86;
    regs.h.al = 0;
    int86(0x15, &regs, &regs);
    outportb(DSP_RESET_AD, 0);*/
    unsigned int resetAd = DSP_RESET_AD;
    _asm {
        mov dx, resetAd;
        mov al, 1;
        out dx, al;
    
        mov cx,0;
        mov dx,3;
        mov ah,86h;
        mov al, 0;
        int 15h;

        mov dx, resetAd;
        mov al, 0;
        out dx, al;
    }
}

/* ************* Public *************** */

ErrorState SBlaster_initialize(void) {
    int unsigned i;
    int status, data;
    ErrorState error;

    error = parseEnvironment();
    if(error != ERR_SUCCESS) {
        return ERR_HARDWARE_NOT_FOUND;
    }

    resetDSP();
    
    for(i= 0; i<65535; ++i) {
        status = 0x80 & inportb(DSP_READ_STATUS_AD);
        if(status != 0) {
            data = inportb(DSP_READ_AD);
            if(data == 0xAA) {
                unsigned char* temp = (unsigned char*)calloc(1, 1);
                unsigned long addr = FP_SEG(temp) * 16 + FP_OFF(temp);
                unsigned long count = 0;
                bufferSize = 1;
                /* increment until page boundary and get max size that way */
                while((addr & 0xFFFF) != 0) {
                    count++;
                    addr = FP_SEG(temp+count) * 16 + FP_OFF(temp+count);
                }
                free(temp);
                /* get previous power of 2 */
                while(bufferSize <= count) {
                    bufferSize <<= 1;
                }
                bufferSize >>= 1;

                /* cap buffer size */
                if(bufferSize > MAX_BUFFER) {
                    bufferSize = MAX_BUFFER;
                }
                halfBufferSize = bufferSize>>1;

                dmaBuffer = (unsigned char*)calloc(bufferSize, sizeof(unsigned char));
                if(!dmaBuffer) {
                    return ERR_OUT_OF_MEMORY;
                }
                return ERR_SUCCESS;
            }
        }
    }

    return ERR_HARDWARE_FAIL;
}

void SBlaster_playWAV(WAVFile file) {
    unsigned int bufferActualLength;

    setISR();

    writeDSP(DMA_STOP);
    bufferActualLength = file->dataLength < bufferSize ? file->dataLength : bufferSize;
    memcpy(dmaBuffer, file->data, bufferActualLength);
    accumulator = 0;
    accumulatorMax = file->dataLength-1;
    dataStart = file->data;
    toggle = 0;

    speakerSet(1);
    setDMA(bufferActualLength < bufferSize ? DMAMode_SINGLE_8BIT : DMAMode_AUTO_8BIT, bufferActualLength);
    setTime(file);
    if(bufferActualLength < bufferSize) {
        setBlockSize(bufferActualLength);
    }
    else {
        setBlockTransferSize();
        writeDSP(outportbUT_8b_AUTO_INIT);
    }
}

void SBlaster_terminate(void) {
    writeDSP(DMA_STOP);
    speakerSet(0);
    resetDSP();
    if(dmaBuffer) {
        free(dmaBuffer);
        dmaBuffer = 0;
    }
}

void waitForMPU() {
    while(inportb(MPU_STATUS_AD) & 0x40 != 0) {

    }
}

unsigned int SBlaster_baseAddress(void) {
    return baseAddr;
}

unsigned char SBlaster_isPlaying(void) {
    return ISRSet;
}
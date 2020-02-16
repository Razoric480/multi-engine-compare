/*

    C++ player code for Reality Adlib Tracker 2.0a (file version 2.1).

    Please note, this is just the player code.  This does no checking of the tune data before
    it tries to play it, as most use cases will be a known tune being used in a production.
    So if you're writing an application that loads unknown tunes in at run time then you'll
    want to do more validity checking.

    To use:

        - Instantiate the RADPlayer object

        - Initialise player for your tune by calling the Init() method.  Supply a pointer to the
          tune file and a function for writing to the OPL3 registers.

        - Call the Update() method a number of times per second as returned by GetHertz().  If
          your tune is using the default BPM setting you can safely just call it 50 times a
          second, unless it's a legacy "slow-timer" tune then it'll need to be 18.2 times a
          second.

        - When you're done, stop calling Update() and call the Stop() method to turn off all
          sound and reset the OPL3 hardware.

*/

#define RADPLAY_IMPORT
#include "audio\RADPlay.h"

/* Various constants*/
enum {
    kTracks = 100,
    kChannels = 9,
    kTrackLines = 64,
    kRiffTracks = 10,
    kInstruments = 127,
    cmPortamentoUp = 0x1,
    cmPortamentoDwn = 0x2,
    cmToneSlide = 0x3,
    cmToneVolSlide = 0x5,
    cmVolSlide = 0xA,
    cmSetVol = 0xC,
    cmJumpToLine = 0xD,
    cmSetSpeed = 0xF,
    cmIgnore = ('I' - 55),
    cmMultiplier = ('M' - 55),
    cmRiff = ('R' - 55),
    cmTranspose = ('T' - 55),
    cmFeedback = ('U' - 55),
    cmVolume = ('V' - 55),
};

typedef enum e_Source {
    SNone, SRiff, SIRiff,
} e_Source;

enum {
    fKeyOn = 1 << 0,
    fKeyOff = 1 << 1,
    fKeyedOn = 1 << 2,
};

typedef struct CInstrument {
    unsigned char Feedback[2];
    unsigned char Panning[2];
    unsigned char Algorithm;
    unsigned char Detune;
    unsigned char Volume;
    unsigned char RiffSpeed;
    unsigned char *Riff;
    unsigned char Operators[4][5];
} CInstrument;

typedef struct CEffects {
    char PortSlide;
    char VolSlide;
    unsigned int ToneSlideFreq;
    unsigned char ToneSlideOct;
    unsigned char ToneSlideSpeed;
    char ToneSlideDir;
} CEffects;

typedef struct CRiff {
    CEffects FX;
    unsigned char *Track;
    unsigned char *TrackStart;
    unsigned char Line;
    unsigned char Speed;
    unsigned char SpeedCnt;
    char TransposeOctave;
    char TransposeNote;
    unsigned char LastInstrument;
} CRiff;

typedef struct CChannel {
    unsigned char LastInstrument;
    CInstrument *Instrument;
    unsigned char Volume;
    unsigned char DetuneA;
    unsigned char DetuneB;
    unsigned char KeyFlags;
    unsigned int CurrFreq;
    char CurrOctave;
    CEffects FX;
    CRiff Riff;
    CRiff IRiff;
} CChannel;

static int Hertz;
static unsigned long PlayTime;
static unsigned char Order;
static unsigned char *OrderList;
static unsigned char OrderListSize;
static unsigned char Line;
static unsigned char MasterVol;
static unsigned char Speed;
static unsigned char UnpackNote(unsigned char **s, unsigned char *last_instrument);
static unsigned char *GetTrack();
static unsigned char *SkipToLine(unsigned char *trk, unsigned char linenum, unsigned char chan_riff);
static void PlayLine();
static void PlayNote(int channum, char notenum, char octave, unsigned int instnum, unsigned char cmd, unsigned char param, e_Source src, int op);
static void LoadInstrumentOPL3(int channum);
static void PlayNoteOPL3(int channum, char octave, char note);
static void ResetFX(CEffects *fx);
static void TickRiff(int channum, CRiff *riff, unsigned char chan_riff);
static void ContinueFX(int channum, CEffects *fx);
static void SetVolume(int channum, unsigned char vol);
static void GetSlideDir(int channum, CEffects *fx);
static void LoadInstMultiplierOPL3(int channum, int op, unsigned char mult);
static void LoadInstVolumeOPL3(int channum, int op, unsigned char vol);
static void LoadInstFeedbackOPL3(int channum, int which, unsigned char fb);
static void Portamento(unsigned int channum, CEffects *fx, char amount, unsigned char toneslide);
static void Transpose(char note, char octave);
static void SetOPL3(unsigned int reg, unsigned char val);
static unsigned char GetOPL3(unsigned int reg);

static void (*OPL3)(void *, unsigned int, unsigned char);
static unsigned char OPL3Regs[512];
static void *OPL3Arg;

static CInstrument Instruments[kInstruments];
static CChannel Channels[kChannels];
        
#if RAD_DETECT_REPEATS
    static unsigned long OrderMap[4];
    static unsigned char Repeating;
#endif

static unsigned char *Tracks[kTracks];
static unsigned char *Riffs[kRiffTracks][kChannels];
static unsigned char *Track;
static unsigned char Initialised;
static unsigned char SpeedCnt;
static char Entrances;
static char LineJump;
/* Values exported by UnpackNote()*/
static char NoteNum;
static char OctaveNum;
static unsigned char InstNum;
static unsigned char EffectNum;
static unsigned char Param;
static unsigned char LastNote;

/*--------------------------------------------------------------------------------------------------*/
static const char NoteSize[] = { 0, 2, 1, 3, 1, 3, 2, 4 };
static const unsigned int ChanOffsets3[9] = { 0, 1, 2, 0x100, 0x101, 0x102, 6, 7, 8 };              /* OPL3 first channel*/
static const unsigned int Chn2Offsets3[9] = { 3, 4, 5, 0x103, 0x104, 0x105, 0x106, 0x107, 0x108 };  /* OPL3 second channel*/
static const unsigned int NoteFreq[] = { 0x16b,0x181,0x198,0x1b0,0x1ca,0x1e5,0x202,0x220,0x241,0x263,0x287,0x2ae };
static const unsigned int OpOffsets3[9][4] = {
    {  0x00B, 0x008, 0x003, 0x000  },
    {  0x00C, 0x009, 0x004, 0x001  },
    {  0x00D, 0x00A, 0x005, 0x002  },
    {  0x10B, 0x108, 0x103, 0x100  },
    {  0x10C, 0x109, 0x104, 0x101  },
    {  0x10D, 0x10A, 0x105, 0x102  },
    {  0x113, 0x110, 0x013, 0x010  },
    {  0x114, 0x111, 0x014, 0x011  },
    {  0x115, 0x112, 0x015, 0x012  }
};
static const unsigned char AlgCarriers[7][4] = {
    {  1, 0, 0, 0 },  /* 0 - 2op - op < op*/
    {  1, 1, 0, 0 },  /* 1 - 2op - op + op*/
    {  1, 0, 0, 0 },  /* 2 - 4op - op < op < op < op*/
    {  1, 0, 0, 1 },  /* 3 - 4op - op < op < op + op*/
    {  1, 0, 1, 0 },  /* 4 - 4op - op < op + op < op*/
    {  1, 0, 1, 1 },  /* 5 - 4op - op < op + op + op*/
    {  1, 1, 1, 1 },  /* 6 - 4op - op + op + op + op*/
};

static void SetOPL3(unsigned int reg, unsigned char val) {
    OPL3Regs[reg] = val;
    OPL3(OPL3Arg, reg, val);
}

static unsigned char GetOPL3(unsigned int reg) {
    return OPL3Regs[reg];
}

/*==================================================================================================*/
/* Unpacks a single RAD note.*/
/*==================================================================================================*/
static unsigned char UnpackNote(unsigned char **s, unsigned char *last_instrument) {

    unsigned char chanid = *(*s)++, note;

    InstNum   = 0;
    EffectNum = 0;
    Param     = 0;

    /* Unpack note data*/
    note = 0;
    if (chanid & 0x40) {
        unsigned char n = *(*s)++;
        note = n & 0x7F;

        /* Retrigger last instrument?*/
        if (n & 0x80)
            InstNum = *last_instrument;
    }

    /* Do we have an instrument?*/
    if (chanid & 0x20) {
        InstNum = *(*s)++;
        *last_instrument = InstNum;
    }

    /* Do we have an effect?*/
    if (chanid & 0x10) {
        EffectNum = *(*s)++;
        Param = *(*s)++;
    }

    NoteNum = note & 15;
    OctaveNum = note >> 4;

    return ((chanid & 0x80) != 0);
}



/*==================================================================================================*/
/* Get current track as indicated by order list.*/
/*==================================================================================================*/
static unsigned char *GetTrack() {
    unsigned char track_num;

    /* If at end of tune start again from beginning*/
    if (Order >= OrderListSize)
        Order = 0;

    track_num = OrderList[Order];

    /* Jump marker?  Note, we don't recognise multiple jump markers as that could put us into an*/
    /* infinite loop*/
    if (track_num & 0x80) {
        Order = track_num & 0x7F;
        track_num = OrderList[Order] & 0x7F;
    }

#if RAD_DETECT_REPEATS
    /* Check for tune repeat, and mark order in order map*/
    if (Order < 128) {
        int byte = Order >> 5;
        unsigned long bit = (unsigned long)(1) << (Order & 31);
        if (OrderMap[byte] & bit)
            Repeating = 1;
        else
            OrderMap[byte] |= bit;
    }
#endif

    return Tracks[track_num];
}



/*==================================================================================================*/
/* Skip through track till we reach the given line or the next higher one.  Returns null if none.*/
/*==================================================================================================*/
static unsigned char *SkipToLine(unsigned char *trk, unsigned char linenum, unsigned char chan_riff) {

    while (1) {

        unsigned char lineid = *trk, chanid;
        if ((lineid & 0x7F) >= linenum)
            return trk;
        if (lineid & 0x80)
            break;
        trk++;

        /* Skip channel notes*/
        do {
            chanid = *trk++;
            trk += NoteSize[(chanid >> 4) & 7];
        } while (!(chanid & 0x80) && !chan_riff);
    }

    return 0;
}



/*==================================================================================================*/
/* Plays one line of current track and advances pointers.*/
/*==================================================================================================*/
static void PlayLine() {
    int i;
    unsigned char *trk;

    SpeedCnt--;
    if (SpeedCnt > 0)
        return;
    SpeedCnt = Speed;

    /* Reset channel effects*/
    for (i = 0; i < kChannels; i++)
        ResetFX(&Channels[i].FX);

    LineJump = -1;

    /* At the right line?*/
    trk = Track;
    if (trk && (*trk & 0x7F) <= Line) {
        unsigned char lineid = *trk++;

        /* Run through channels*/
        unsigned char last;
        do {
            int channum = *trk & 15;
            CChannel *chan = &Channels[channum];
            last = UnpackNote(&trk, &chan->LastInstrument);
            PlayNote(channum, NoteNum, OctaveNum, InstNum, EffectNum, Param, SNone, 0);
        } while (!last);

        /* Was this the last line?*/
        if (lineid & 0x80)
            trk = 0;

        Track = trk;
    }

    /* Move to next line*/
    Line++;
    if (Line >= kTrackLines || LineJump >= 0) {

        if (LineJump >= 0)
            Line = LineJump;
        else
            Line = 0;

        /* Move to next track in order list*/
        Order++;
        Track = GetTrack();
    }
}



/*==================================================================================================*/
/* Play a single note.  Returns the line number in the next pattern to jump to if a jump command was*/
/* found, or -1 if none.*/
/*==================================================================================================*/
static void PlayNote(int channum, char notenum, char octave, unsigned int instnum, unsigned char cmd, unsigned char param, e_Source src, int op) {
    CChannel *chan = &Channels[channum];
    unsigned char transposing;
    CEffects* fx;

    /* Recursion detector.  This is needed as riffs can trigger other riffs, and they could end up*/
    /* in a loop*/
    if (Entrances >= 8)
        return;
    Entrances++;

    /* Select which effects source we're using*/
    fx = &chan->FX;
    if (src == SRiff)
        fx = &chan->Riff.FX;
    else if (src == SIRiff)
        fx = &chan->IRiff.FX;

    transposing = 0;

    /* For tone-slides the note is the target*/
    if (cmd == cmToneSlide) {
        if (notenum > 0 && notenum <= 12) {
            fx->ToneSlideOct = octave;
            fx->ToneSlideFreq = NoteFreq[notenum - 1];
        }
        goto toneslide;
    }

    /* Playing a new instrument?*/
    if (instnum > 0) {
        CInstrument *oldinst = chan->Instrument;
        CInstrument *inst = &Instruments[instnum - 1];
        chan->Instrument = inst;

        /* Ignore MIDI instruments*/
        if (inst->Algorithm == 7) {
            Entrances--;
            return;
        }

        LoadInstrumentOPL3(channum);

        /* Bounce the channel*/
        chan->KeyFlags |= fKeyOff | fKeyOn;

        ResetFX(&chan->IRiff.FX);

        if (src != SIRiff || inst != oldinst) {

            /* Instrument riff?*/
            if (inst->Riff && inst->RiffSpeed > 0) {

                chan->IRiff.Track = chan->IRiff.TrackStart = inst->Riff;
                chan->IRiff.Line = 0;
                chan->IRiff.Speed = inst->RiffSpeed;
                chan->IRiff.LastInstrument = 0;

                /* Note given with riff command is used to transpose the riff*/
                if (notenum >= 1 && notenum <= 12) {
                    chan->IRiff.TransposeOctave = octave;
                    chan->IRiff.TransposeNote = notenum;
                    transposing = 1;
                } else {
                    chan->IRiff.TransposeOctave = 3;
                    chan->IRiff.TransposeNote = 12;
                }

                /* Do first tick of riff*/
                chan->IRiff.SpeedCnt = 1;
                TickRiff(channum, &chan->IRiff, 0);

            } else
                chan->IRiff.SpeedCnt = 0;
        }
    }

    /* Starting a channel riff?*/
    if (cmd == cmRiff || cmd == cmTranspose) {
        unsigned char p0, p1;

        ResetFX(&chan->Riff.FX);

        p0 = param / 10;
        p1 = param % 10;
        chan->Riff.Track = p1 > 0 ? Riffs[p0][p1 - 1] : 0;
        if (chan->Riff.Track) {

            chan->Riff.TrackStart = chan->Riff.Track;
            chan->Riff.Line = 0;
            chan->Riff.Speed = Speed;
            chan->Riff.LastInstrument = 0;

            /* Note given with riff command is used to transpose the riff*/
            if (cmd == cmTranspose && notenum >= 1 && notenum <= 12) {
                chan->Riff.TransposeOctave = octave;
                chan->Riff.TransposeNote = notenum;
                transposing = 1;
            } else {
                chan->Riff.TransposeOctave = 3;
                chan->Riff.TransposeNote = 12;
            }

            /* Do first tick of riff*/
            chan->Riff.SpeedCnt = 1;
            TickRiff(channum, &chan->Riff, 1);

        } else
            chan->Riff.SpeedCnt = 0;
    }

    /* Play the note*/
    if (!transposing && notenum > 0) {

        /* Key-off?*/
        if (notenum == 15)
            chan->KeyFlags |= fKeyOff;

        if (!chan->Instrument || chan->Instrument->Algorithm < 7)
            PlayNoteOPL3(channum, octave, notenum);
    }

    /* Process effect*/
    switch (cmd) {

        case cmSetVol:
            SetVolume(channum, param);
            break;

        case cmSetSpeed:
            if (src == SNone) {
                Speed = param;
                SpeedCnt = param;
            } else if (src == SRiff) {
                chan->Riff.Speed = param;
                chan->Riff.SpeedCnt = param;
            } else if (src == SIRiff) {
                chan->IRiff.Speed = param;
                chan->IRiff.SpeedCnt = param;
            }
            break;

        case cmPortamentoUp:
            fx->PortSlide = param;
            break;

        case cmPortamentoDwn:
            fx->PortSlide = -(char)(param);
            break;

        case cmToneVolSlide:
        case cmVolSlide: {
            char val = param;
            if (val >= 50)
                val = -(val - 50);
            fx->VolSlide = val;
            if (cmd != cmToneVolSlide)
                break;
        }
        /* Fall through!*/

        case cmToneSlide: {
            unsigned char speed;
toneslide:
            speed = param;
            if (speed)
                fx->ToneSlideSpeed = speed;
            GetSlideDir(channum, fx);
            break;
        }

        case cmJumpToLine: {
            if (param >= kTrackLines)
                break;

            /* Note: jump commands in riffs are checked for within TickRiff()*/
            if (src == SNone)
                LineJump = param;

            break;
        }

        case cmMultiplier: {
            if (src == SIRiff)
                LoadInstMultiplierOPL3(channum, op, param);
            break;
        }

        case cmVolume: {
            if (src == SIRiff)
                LoadInstVolumeOPL3(channum, op, param);
            break;
        }

        case cmFeedback: {
            if (src == SIRiff) {
                unsigned char which = param / 10;
                unsigned char fb = param % 10;
                LoadInstFeedbackOPL3(channum, which, fb);
            }
            break;
        }
    }

    Entrances--;
}



/*==================================================================================================*/
/* Sets the OPL3 registers for a given instrument.*/
/*==================================================================================================*/
static void LoadInstrumentOPL3(int channum) {
    CChannel *chan = &Channels[channum];
    unsigned char alg;
    int i;

    const CInstrument *inst = chan->Instrument;
    if (!inst)
        return;

    alg = inst->Algorithm;
    chan->Volume = inst->Volume;
    chan->DetuneA = (inst->Detune + 1) >> 1;
    chan->DetuneB = inst->Detune >> 1;

    /* Turn on 4-op mode for algorithms 2 and 3 (algorithms 4 to 6 are simulated with 2-op mode)*/
    if (channum < 6) {
        unsigned char mask = 1 << channum;
        SetOPL3(0x104, (GetOPL3(0x104) & ~mask) | (alg == 2 || alg == 3 ? mask : 0));
    }

    /* Left/right/feedback/algorithm*/
    SetOPL3(0xC0 + ChanOffsets3[channum], ((inst->Panning[1] ^ 3) << 4) | inst->Feedback[1] << 1 | (alg == 3 || alg == 5 || alg == 6 ? 1 : 0));
    SetOPL3(0xC0 + Chn2Offsets3[channum], ((inst->Panning[0] ^ 3) << 4) | inst->Feedback[0] << 1 | (alg == 1 || alg == 6 ? 1 : 0));

    /* Load the operators*/
    for (i = 0; i < 4; i++) {

        static const unsigned char blank[] = { 0, 0x3F, 0, 0xF0, 0 };
        const unsigned char *op = (alg < 2 && i >= 2) ? blank : inst->Operators[i];
        unsigned int reg = OpOffsets3[channum][i];

        unsigned int vol = ~op[1] & 0x3F;

        /* Do volume scaling for carriers*/
        if (AlgCarriers[alg][i]) {
            vol = vol * inst->Volume / 64;
            vol = vol * MasterVol / 64;
        }

        SetOPL3(reg + 0x20, op[0]);
        SetOPL3(reg + 0x40, (op[1] & 0xC0) | ((vol ^ 0x3F) & 0x3F));
        SetOPL3(reg + 0x60, op[2]);
        SetOPL3(reg + 0x80, op[3]);
        SetOPL3(reg + 0xE0, op[4]);
    }
}



/*==================================================================================================*/
/* Play note on OPL3 hardware.*/
/*==================================================================================================*/
static void PlayNoteOPL3(int channum, char octave, char note) {
    CChannel *chan = &Channels[channum];

    unsigned int o1 = ChanOffsets3[channum];
    unsigned int o2 = Chn2Offsets3[channum];

    unsigned char op4;
    unsigned int freq, frq2;

    /* Key off the channel*/
    if (chan->KeyFlags & fKeyOff) {
        chan->KeyFlags &= ~(fKeyOff | fKeyedOn);
        SetOPL3(0xB0 + o1, GetOPL3(0xB0 + o1) & ~0x20);
        SetOPL3(0xB0 + o2, GetOPL3(0xB0 + o2) & ~0x20);
    }

    if (note == 15)
        return;

    op4 = (chan->Instrument && chan->Instrument->Algorithm >= 2);

    freq = NoteFreq[note - 1];
    frq2 = freq;

    chan->CurrFreq = freq;
    chan->CurrOctave = octave;

    /* Detune.  We detune both channels in the opposite direction so the note retains its tuning*/
    freq += chan->DetuneA;
    frq2 -= chan->DetuneB;

    /* Frequency low byte*/
    if (op4)
        SetOPL3(0xA0 + o1, frq2 & 0xFF);
    SetOPL3(0xA0 + o2, freq & 0xFF);

    /* Frequency high bits + octave + key on*/
    if (chan->KeyFlags & fKeyOn)
        chan->KeyFlags = (chan->KeyFlags & ~fKeyOn) | fKeyedOn;
    if (op4)
        SetOPL3(0xB0 + o1, (frq2 >> 8) | (octave << 2) | ((chan->KeyFlags & fKeyedOn) ? 0x20 : 0));
    else
        SetOPL3(0xB0 + o1, 0);
    SetOPL3(0xB0 + o2, (freq >> 8) | (octave << 2) | ((chan->KeyFlags & fKeyedOn) ? 0x20 : 0));
}



/*==================================================================================================*/
/* Prepare FX for new line.*/
/*==================================================================================================*/
static void ResetFX(CEffects *fx) {
    fx->PortSlide = 0;
    fx->VolSlide = 0;
    fx->ToneSlideDir = 0;
}

/*==================================================================================================*/
/* Tick the channel riff.*/
/*==================================================================================================*/
static void TickRiff(int channum, CRiff *riff, unsigned char chan_riff) {
    unsigned char lineid, line, *trk;

    if (riff->SpeedCnt == 0) {
        ResetFX(&riff->FX);
        return;
    }

    riff->SpeedCnt--;
    if (riff->SpeedCnt > 0)
        return;
    riff->SpeedCnt = riff->Speed;

    line = riff->Line++;
    if (riff->Line >= kTrackLines)
        riff->SpeedCnt = 0;

    ResetFX(&riff->FX);

    /* Is this the current line in track?*/
    trk = riff->Track;
    if (trk && (*trk & 0x7F) == line) {
        lineid = *trk++;

        if (chan_riff) {

            /* Channel riff: play current note*/
            UnpackNote(&trk, &riff->LastInstrument);
            Transpose(riff->TransposeNote, riff->TransposeOctave);
            PlayNote(channum, NoteNum, OctaveNum, InstNum, EffectNum, Param, SRiff, 0);

        } else {

            /* Instrument riff: here each track channel is an extra effect that can run, but is not*/
            /* actually a different physical channel*/
            unsigned char last;
            do {
                int col = *trk & 15;
                last = UnpackNote(&trk, &riff->LastInstrument);
                if (EffectNum != cmIgnore)
                    Transpose(riff->TransposeNote, riff->TransposeOctave);
                PlayNote(channum, NoteNum, OctaveNum, InstNum, EffectNum, Param, SIRiff, col > 0 ? (col - 1) & 3 : 0);
            } while (!last);
        }

        /* Last line?*/
        if (lineid & 0x80)
            trk = 0;

        riff->Track = trk;
    }

    /* Special case; if next line has a jump command, run it now*/
    if (!trk || (*trk++ & 0x7F) != riff->Line)
        return;

    UnpackNote(&trk, &lineid); /* lineid is just a dummy here*/
    if (EffectNum == cmJumpToLine && Param < kTrackLines) {
        riff->Line = Param;
        riff->Track = SkipToLine(riff->TrackStart, Param, chan_riff);
    }
}



/*==================================================================================================*/
/* This continues any effects that operate continuously (eg. slides).*/
/*==================================================================================================*/
static void ContinueFX(int channum, CEffects *fx) {
    CChannel *chan = &Channels[channum];

    if (fx->PortSlide)
        Portamento(channum, fx, fx->PortSlide, 0);

    if (fx->VolSlide) {
        char vol = chan->Volume;
        vol -= fx->VolSlide;
        if (vol < 0)
            vol = 0;
        SetVolume(channum, vol);
    }

    if (fx->ToneSlideDir)
        Portamento(channum, fx, fx->ToneSlideDir, 1);
}



/*==================================================================================================*/
/* Sets the volume of given channel.*/
/*==================================================================================================*/
static void SetVolume(int channum, unsigned char vol) {
    CChannel *chan = &Channels[channum];
    CInstrument *inst;
    unsigned char alg;
    int i;

    /* Ensure volume is within range*/
    if (vol > 64)
        vol = 64;

    chan->Volume = vol;

    /* Scale volume to master volume*/
    vol = vol * MasterVol / 64;

    inst = chan->Instrument;
    if (!inst)
        return;
    alg = inst->Algorithm;

    /* Set volume of all carriers*/
    for (i = 0; i < 4; i++) {
        unsigned char *op = inst->Operators[i], opvol;
        unsigned int reg;

        /* Is this operator a carrier?*/
        if (!AlgCarriers[alg][i])
            continue;

        opvol = (unsigned int)((op[1] & 63) ^ 63) * vol / 64;
        reg = 0x40 + OpOffsets3[channum][i];
        SetOPL3(reg, (GetOPL3(reg) & 0xC0) | (opvol ^ 0x3F));
    }
}



/*==================================================================================================*/
/* Starts a tone-slide.*/
/*==================================================================================================*/
static void GetSlideDir(int channum, CEffects *fx) {
    CChannel *chan = &Channels[channum];

    char speed = fx->ToneSlideSpeed;
    if (speed > 0) {
        unsigned char oct = fx->ToneSlideOct;
        unsigned int freq = fx->ToneSlideFreq;

        unsigned int oldfreq = chan->CurrFreq;
        unsigned char oldoct = chan->CurrOctave;

        if (oldoct > oct)
            speed = -speed;
        else if (oldoct == oct) {
            if (oldfreq > freq)
                speed = -speed;
            else if (oldfreq == freq)
                speed = 0;
        }
    }

    fx->ToneSlideDir = speed;
}

/*==================================================================================================*/
/* Load multiplier value into operator.*/
/*==================================================================================================*/
static void LoadInstMultiplierOPL3(int channum, int op, unsigned char mult) {
    unsigned int reg = 0x20 + OpOffsets3[channum][op];
    SetOPL3(reg, (GetOPL3(reg) & 0xF0) | (mult & 15));
}



/*==================================================================================================*/
/* Load volume value into operator.*/
/*==================================================================================================*/
static void LoadInstVolumeOPL3(int channum, int op, unsigned char vol) {
    unsigned int reg = 0x40 + OpOffsets3[channum][op];
    SetOPL3(reg, (GetOPL3(reg) & 0xC0) | ((vol & 0x3F) ^ 0x3F));
}



/*==================================================================================================*/
/* Load feedback value into instrument.*/
/*==================================================================================================*/
static void LoadInstFeedbackOPL3(int channum, int which, unsigned char fb) {

    if (which == 0) {

        unsigned int reg = 0xC0 + Chn2Offsets3[channum];
        SetOPL3(reg, (GetOPL3(reg) & 0x31) | ((fb & 7) << 1));

    } else if (which == 1) {

        unsigned int reg = 0xC0 + ChanOffsets3[channum];
        SetOPL3(reg, (GetOPL3(reg) & 0x31) | ((fb & 7) << 1));
    }
}



/*==================================================================================================*/
/* This adjusts the pitch of the given channel's note.  There may also be a limiting value on the*/
/* portamento (for tone slides).*/
/*==================================================================================================*/
static void Portamento(unsigned int channum, CEffects *fx, char amount, unsigned char toneslide) {
    CChannel *chan = &Channels[channum];

    unsigned int freq = chan->CurrFreq, frq2, chan_offset;
    unsigned char oct = chan->CurrOctave;

    freq += amount;

    if (freq < 0x156) {

        if (oct > 0) {
            oct--;
            freq += 0x2AE - 0x156;
        } else
            freq = 0x156;

    } else if (freq > 0x2AE) {

        if (oct < 7) {
            oct++;
            freq -= 0x2AE - 0x156;
        } else
            freq = 0x2AE;
    }

    if (toneslide) {

        if (amount >= 0) {

            if (oct > fx->ToneSlideOct || (oct == fx->ToneSlideOct && freq >= fx->ToneSlideFreq)) {
                freq = fx->ToneSlideFreq;
                oct = fx->ToneSlideOct;
            }

        } else {

            if (oct < fx->ToneSlideOct || (oct == fx->ToneSlideOct && freq <= fx->ToneSlideFreq)) {
                freq = fx->ToneSlideFreq;
                oct = fx->ToneSlideOct;
            }
        }
    }

    chan->CurrFreq = freq;
    chan->CurrOctave = oct;

    /* Apply detunes*/
    frq2 = freq - chan->DetuneB;
    freq += chan->DetuneA;

    /* Write value back to OPL3*/
    chan_offset = Chn2Offsets3[channum];
    SetOPL3(0xA0 + chan_offset, freq & 0xFF);
    SetOPL3(0xB0 + chan_offset, (freq >> 8 & 3) | oct << 2 | (GetOPL3(0xB0 + chan_offset) & 0xE0));

    chan_offset = ChanOffsets3[channum];
    SetOPL3(0xA0 + chan_offset, frq2 & 0xFF);
    SetOPL3(0xB0 + chan_offset, (frq2 >> 8 & 3) | oct << 2 | (GetOPL3(0xB0 + chan_offset) & 0xE0));
}

/*==================================================================================================*/
/* Transpose the note returned by UnpackNote().*/
/* Note: due to RAD's wonky legacy middle C is octave 3 note number 12.*/
/*==================================================================================================*/
static void Transpose(char note, char octave) {

    if (NoteNum >= 1 && NoteNum <= 12) {

        char toct = octave - 3, tnot;
        if (toct != 0) {
            OctaveNum += toct;
            if (OctaveNum < 0)
                OctaveNum = 0;
            else if (OctaveNum > 7)
                OctaveNum = 7;
        }

        tnot = note - 12;
        if (tnot != 0) {
            NoteNum += tnot;
            if (NoteNum < 1) {
                NoteNum += 12;
                if (OctaveNum > 0)
                    OctaveNum--;
                else
                    NoteNum = 1;
            }
        }
    }
}

/*==================================================================================================*/
/* Initialise a RAD tune for playback.  This assumes the tune data is valid and does minimal data*/
/* checking.*/
/*==================================================================================================*/
ErrorState RADPlayer_Init(const void *tune, void (*opl3)(void *, unsigned int, unsigned char), void *arg) {
    unsigned char *s, flags;
    int i, j;

    Initialised = 0;

    /* Version check; we only support version 2.1 tune files*/
    if (*((unsigned char *)tune + 0x10) != 0x21) {
        Hertz = -1;
        return ERR_WRONG_FILE_FORMAT;
    }

    /* The OPL3 call-back*/
    OPL3 = opl3;
    OPL3Arg = arg;

    for (i = 0; i < kTracks; i++)
        Tracks[i] = 0;

    for (i = 0; i < kRiffTracks; i++)
        for (j = 0; j < kChannels; j++)
            Riffs[i][j] = 0;

    s = (unsigned char *)tune + 0x11;

    flags = *s++;
    Speed = flags & 0x1F;

    /* Is BPM value present?*/
    Hertz = 50;
    if (flags & 0x20) {
        Hertz = (s[0] | ((int)(s[1]) << 8)) * 2 / 5;
        s += 2;
    }

    /* Slow timer tune?  Return an approximate hz*/
    if (flags & 0x40)
        Hertz = 18;

    /* Skip any description*/
    while (*s)
        s++;
    s++;

    /* Unpack the instruments*/
    while (1) {

        /* Instrument number, 0 indicates end of list*/
        unsigned char inst_num = *s++, alg;
        CInstrument *inst;
        if (inst_num == 0)
            break;

        /* Skip instrument name*/
        s += *s++;

        inst = &Instruments[inst_num - 1];

        alg = *s++;
        inst->Algorithm = alg & 7;
        inst->Panning[0] = (alg >> 3) & 3;
        inst->Panning[1] = (alg >> 5) & 3;

        if (inst->Algorithm < 7) {

            unsigned char b = *s++;
            inst->Feedback[0] = b & 15;
            inst->Feedback[1] = b >> 4;

            b = *s++;
            inst->Detune = b >> 4;
            inst->RiffSpeed = b & 15;

            inst->Volume = *s++;

            for (i = 0; i < 4; i++) {
                unsigned char *op = inst->Operators[i];
                for (j = 0; j < 5; j++)
                    op[j] = *s++;
            }

        } else {

            /* Ignore MIDI instrument data*/
            s += 6;
        }

        /* Instrument riff?*/
        if (alg & 0x80) {
            int size = s[0] | ((int)(s[1]) << 8);
            s += 2;
            inst->Riff = s;
            s += size;
        } else
            inst->Riff = 0;
    }

    /* Get order list*/
    OrderListSize = *s++;
    OrderList = s;
    s += OrderListSize;

    /* Locate the tracks*/
    while (1) {

        /* Track number*/
        unsigned char track_num = *s++;
        int size;
        if (track_num >= kTracks)
            break;

        /* Track size in bytes*/
        size = s[0] | ((int)(s[1]) << 8);
        s += 2;

        Tracks[track_num] = s;
        s += size;
    }

    /* Locate the riffs*/
    while (1) {

        /* Riff id*/
        unsigned char riffid = *s++;
        unsigned char riffnum = riffid >> 4;
        unsigned char channum = riffid & 15;
        int size;
        if (riffnum >= kRiffTracks || channum > kChannels)
            break;

        /* Track size in bytes*/
        size = s[0] | ((int)(s[1]) << 8);
        s += 2;

        Riffs[riffnum][channum - 1] = s;
        s += size;
    }

    /* Done parsing tune, now set up for play*/
    for (i = 0; i < 512; i++)
        OPL3Regs[i] = 255;
    
    Initialised = 1;
    RADPlayer_Stop();
}



/*==================================================================================================*/
/* Stop all sounds and reset the tune.  Tune will play from the beginning again if you continue to*/
/* Update().*/
/*==================================================================================================*/
void RADPlayer_Stop() {
    unsigned int reg;
    int i;

    if(!Initialised) {
        return;
    }
    
    /* Clear all registers*/
    for (reg = 0x20; reg < 0xF6; reg++) {
        /* Ensure envelopes decay all the way*/
        unsigned char val = (reg >= 0x60 && reg < 0xA0) ? 0xFF : 0;

        SetOPL3(reg, val);
        SetOPL3(reg + 0x100, val);
    }

    /* Configure OPL3*/
    SetOPL3(1, 0x20);  /* Allow waveforms*/
    SetOPL3(8, 0);     /* No split point*/
    SetOPL3(0xbd, 0);  /* No drums, etc.*/
    SetOPL3(0x104, 0); /* Everything 2-op by default*/
    SetOPL3(0x105, 1); /* OPL3 mode on*/

#if RAD_DETECT_REPEATS
    /* The order map keeps track of which patterns we've played so we can detect when the tune*/
    /* starts to repeat.  Jump markers can't be reliably used for this*/
    PlayTime = 0;
    Repeating = 0;
    for (i = 0; i < 4; i++)
        OrderMap[i] = 0;
#endif

    /* Initialise play values*/
    SpeedCnt = 1;
    Order = 0;
    Track = GetTrack();
    Line = 0;
    Entrances = 0;
    MasterVol = 64;

    /* Initialise channels*/
    for (i = 0; i < kChannels; i++) {
        CChannel *chan = &Channels[i];
        chan->LastInstrument = 0;
        chan->Instrument = 0;
        chan->Volume = 0;
        chan->DetuneA = 0;
        chan->DetuneB = 0;
        chan->KeyFlags = 0;
        chan->Riff.SpeedCnt = 0;
        chan->IRiff.SpeedCnt = 0;
    }
}

int RADPlayer_GetHertz() {
    return Hertz;
}

int RADPlayer_GetPlayTimeInSeconds() {
    return PlayTime / Hertz;
}

int RADPlayer_GetTunePos() {
    return Order;
}

int RADPlayer_GetTuneLength() {
    return OrderListSize;
}

int RADPlayer_GetTuneLine() {
    return Line;
}

void RADPlayer_SetMasterVolume(int vol) {
    MasterVol = vol;
}

int RADPlayer_GetMasterVolume() {
    return MasterVol;
}

int RADPlayer_GetSpeed() {
    return Speed;
}

/*==================================================================================================*/
/* Playback update.  Call BPM * 2 / 5 times a second.  Use GetHertz() for this number after the*/
/* tune has been initialised.  Returns 1 if tune is starting to repeat.*/
/*==================================================================================================*/
unsigned char RADPlayer_Update() {
    int i;

    if (!Initialised)
        return 0;

    /* Run riffs*/
    for (i = 0; i < kChannels; i++) {
        CChannel *chan = &Channels[i];
        TickRiff(i, &chan->IRiff, 0);
        TickRiff(i, &chan->Riff, 1);
    }

    /* Run main track*/
    PlayLine();

    /* Run effects*/
    for (i = 0; i < kChannels; i++) {
        CChannel *chan = &Channels[i];
        ContinueFX(i, &chan->IRiff.FX);
        ContinueFX(i, &chan->Riff.FX);
        ContinueFX(i, &chan->FX);
    }

    /* Update play time.  We convert to seconds when queried*/
    PlayTime++;

#if RAD_DETECT_REPEATS
    return Repeating;
#else
    return 0;
#endif
}

/*==================================================================================================*/
/* Compute total time of tune if it didn't repeat.  Note, this stops the tune so should only be done*/
/* prior to initial playback.*/
/*==================================================================================================*/
#if RAD_DETECT_REPEATS
static void RADPlayerDummyOPL3(void *arg, unsigned int reg, unsigned char data) {
    (void)arg;
    (void)reg;
    (void)data;
}
/*--------------------------------------------------------------------------------------------------*/
unsigned long RADPlayer_ComputeTotalTime() {
    void (*old_opl3)(void *, unsigned int, unsigned char);
    unsigned long total;
    RADPlayer_Stop();
    old_opl3 = OPL3;
    OPL3 = RADPlayerDummyOPL3;

    while (!RADPlayer_Update())
        ;
    total = PlayTime;

    RADPlayer_Stop();
    OPL3 = old_opl3;

    return total / Hertz;
}
#endif

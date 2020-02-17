import path = require("path");
import glob = require("glob");
import fs = require("fs");

export class FileWalker {
    private pathTo: string;

    constructor(pathTo: string) {
        this.pathTo = pathTo;
    }

    getAllFiles(types: string[], callback: (error: Error, matches: string[]) => void) {
        glob(this.pathTo + `/**/*.{${types.join(",")}}`, callback);
    }
    
    buildMake(execPath: string, outPath: string) {
        this.getAllFiles(["c", "C", "asm", "ASM"], (err, matches) => {
            let files: string[] = [];
            if(err) {
                console.error("Error: ", err);
                return;
            }
            files = matches.map(m => {
                return m.replace("../", "").replace(/\//g, "\\");
            });
            
            let runtime = "C:\\TC\\LIB\\C0l";
            let objs: string[] = [`lib\\XLIB611L.lib`];
            let exec = execPath.slice(0, execPath.length-4);
            let libs = ["C:\\TC\\LIB\\emu", "C:\\TC\\LIB\\mathl", "C:\\TC\\LIB\\Cl"];
            let sources: string[] = [];
            let makeObjCommands: string[] = [];
            
            let proj: string[] = [];
            
            files.forEach(f => {
                let ext = path.extname(f);
                let filename = path.basename(f).replace(new RegExp(ext), "");
                objs.push(`obj\\${filename}.obj`);
                makeObjCommands.push(`obj\\${filename}.obj: ${f}\n    tcc -ml -G -nobj -c -Isrc ${f}`);
                sources.push(`obj\\${filename}.obj`);
                proj.push(f);
            });
            
            let sourcesMac = `srcs= \\ \n${sources.join(" \\\n")}`;
            let libsMac = `${libs.join("+\n")}`
            let execMac = exec;
            let objsMac = `${objs.join("+\n")}`;
            let runtimeMac = runtime;
            let projFile = proj.join('\n');
            
            let make = `!include "SOURCES.MAC"\n\n${execPath}: $(srcs)\n    tlink /c /s @RUNTIME @LISTOBJS , @EXEC , @EXEC , @LISTLIBS\n\n`;
            make += makeObjCommands.join("\n\n");
            
            if(outPath.length !== 0 && !fs.existsSync(outPath)) {
                fs.mkdirSync(outPath);
            }
            
            fs.writeFileSync(`${outPath}\\SOURCES.MAC`, sourcesMac);
            fs.writeFileSync(`${outPath}\\LISTLIBS`, libsMac);
            fs.writeFileSync(`${outPath}\\EXEC`, execMac);
            fs.writeFileSync(`${outPath}\\LISTOBJS`, objsMac);
            fs.writeFileSync(`${outPath}\\RUNTIME`, runtimeMac);
            fs.writeFileSync(`${outPath}\\MAKEFILE`, make);
            fs.writeFileSync(`${outPath}\\RZRCA.PRJ`, projFile);
        });
    }
}

import { FileWalker } from "./FileWalker";
import path = require("path");

let walker = new FileWalker("./src");
walker.buildMake("bin\\GAME.EXE", "./");
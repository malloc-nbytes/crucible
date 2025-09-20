#!/usr/bin/env earl

module Runner

import "std/system.rl"; as sys
import "std/colors.rl"; as colors
import "std/io.rl";     as io

set_flag("-x");

enum FlagType {
    Clean = 1 << 0,
    Asm   = 1 << 1,
}

class Config [] { @pub let flags = 0x0000; }

fn usage() {
    println("Usage: earl runner.rl -- [options...]");
    println("Options:");
    println("    asm      show assembly of failed tests");
    println("    clean    remove all generated files");
    exit(0);
}

fn parse_args(@ref config) {
    foreach arg in argv()[1:] {
        if arg == "asm" {
            config.flags `|= FlagType.Asm;
        } else if arg == "clean" {
            config.flags `|= FlagType.Clean;
        } else {
            bad(format("unknown option `", arg, "`"), true);
            usage();
        }
    }
}

fn ok(msg, newline) {
    print(colors::Tfc.Green, colors::Te.Bold, msg, colors::Te.Reset);
    if newline { println(); }
}

fn bad(msg, newline) {
    print(colors::Tfc.Red, colors::Te.Bold, msg, colors::Te.Reset);
    if newline { println(); }
}

fn info(msg, newline) {
    print(colors::Tfc.Yellow, msg, colors::Te.Reset);
    if newline { println(); }
}

fn dump_asm(config, fp) {
    if (config.flags `& FlagType.Asm) == 0 { return; }

    let name = fp.split(".bin")[0];
    let asm = io::file_to_str(name + ".asm");

    println();

    let lines = asm.split("\n");
    for i in 0 to len(lines) {
        if i != len(lines)-1 && len(lines[i]) != 0 {
            println(colors::Te.Bold, "    ", lines[i], colors::Te.Reset);
        }
    }
}

fn cleanup(config) {
    if (config.flags `& FlagType.Clean) == 0 {
        println();
    }

    let bins, asms = (
        sys::get_all_files_by_ext(".", "bin"),
        sys::get_all_files_by_ext(".", "asm"),
    );

    bins.foreach(|f| { $f"rm {f}"; });
    asms.foreach(|f| { $f"rm {f}"; });

    if (config.flags `& FlagType.Clean) != 0 {
        exit(0);
    }
}

fn setup_compiler() {
    $"pwd" |> let cwd;
    cd("..");
    $"earl build.rl";
    cd(cwd);
}

fn compile() {
    let files = sys::get_all_files_by_ext(".", "cru");

    foreach f in files {
        $f"../cruc {f} -o {f}.bin --asm";
    }
}

fn run_tests(config) {
    let files = sys::get_all_files_by_ext(".", "bin");
    let passed, failed = ([], []);

    println();

    foreach f in files {
        let code = sys::cmdstr_wexitcode(f"{f}")[0];

        info(f"Test: {f}", false);
        print(" ... ");

        if code != 0 {
            failed += [f];
            bad(format(colors::Te.Invert, "FAIL"), false);
            dump_asm(config, f);
        } else {
            passed += [f];
            ok("ok", false);
        }

        println();
    }

    ok(format("Passed: ", len(passed)), true);
    bad(format("Failed: ", len(failed)), true);
}

let config = Config();

parse_args(config);
cleanup(config);
setup_compiler();
compile();
run_tests(config);

#!/usr/bin/env earl

module Runner

import "std/system.rl"; as sys
import "std/colors.rl"; as colors
import "std/io.rl";     as io

set_flag("-e");

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
        if arg == "help" {
            usage();
        } else if arg == "asm" {
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
    info("=== CLEANUP ===", 1);

    # if (config.flags `& FlagType.Clean) == 0 {
    #     println();
    # }

    let bins, asms, os = (
        sys::get_all_files_by_ext(".", "bin"),
        sys::get_all_files_by_ext(".", "asm"),
        sys::get_all_files_by_ext(".", "o"),
    );

    foreach b in bins {
        println(f"[RM] {b}");
        $f"rm {b}";
    }

    foreach a in asms {
        println(f"[RM] {a}");
        $f"rm {a}";
    }

    foreach o in os {
        println(f"[RM] {o}");
        $f"rm {o}";
    }

    if (config.flags `& FlagType.Clean) != 0 {
        exit(0);
    }
}

fn setup_compiler() {
    info("=== BUILDING COMPILER ===", 1);
    $"pwd" |> let cwd;
    cd("../../");
    $"make clean";
    $"make";
    cd(cwd);
}

fn compile() {
    info("=== COMPILING TESTS ===", 1);

    let files = sys::get_all_files_by_ext(".", "cr");

    foreach f in files {

        let cmd = f"../../cruc {f} -o {f}.bin --asm";

        println(f"[CC] {cmd}");

        with code = sys::cmdstr_wexitcode(cmd)[0]
        in if code != 0 {
            bad(format(colors::Te.Invert, "compilation error: ", code), 1);
            bad("Aborting...", 1);
            exit(0);
        }
    }
}

fn run_tests(config) {
    info("=== RUNNING TESTS ===", 0);

    let files = sys::get_all_files_by_ext(".", "bin");
    let passed, failed = ([], []);

    println();

    let maxfsz = files.fold(|f, acc| {
        case len(f) > acc of { true = len(f); _ = acc; };
    }, 0);

    foreach f in files {
        let code = sys::cmdstr_wexitcode(f"{f}")[0];

        info(format(colors::Te.Bold, f"[T] {f}"), false);
        for i in 0 to maxfsz-len(f)+1 { print(' '); }

        if code != 0 {
            failed += [f];
            bad(format(colors::Te.Invert, f"FAIL (exited with code {code})" + case code of { 11 = " [segfault]"; _ = ""; }), false);
            dump_asm(config, f);
        } else {
            passed += [f];
            ok("ok", false);
        }

        println();
    }

    ok(format('\n', colors::Te.Invert, "Passed: ", len(passed)), true);
    bad(format(colors::Te.Invert, "Failed: ", len(failed)), true);
}

let config = Config();

parse_args(config);
cleanup(config);
setup_compiler();
compile();
run_tests(config);

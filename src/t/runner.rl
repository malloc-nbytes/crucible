#!/usr/bin/env earl

module Runner

import "std/system.rl"; as sys
import "std/colors.rl"; as colors

# set_flag("-xe");

fn cleanup() {
    $"rm *.bin";
}

fn compile() {
    let files = sys::ls(".").filter(|f| {
            f != "./runner.rl" && f.split(".").rev()[0] != "bin";
    });

    foreach f in files {
        $f"../cruc ./{f} -o {f}.bin";
    }
}

fn run_tests() {
    let files = sys::get_all_files_by_ext(".", "bin");
    let passed, failed = ([], []);

    foreach f in files {
        let code = sys::cmdstr_wexitcode(f"{f}")[0];
        if code != 0 {
            failed += [f];
        } else {
            passed += [f];
        }
    }

    for i in 0 to len(failed) {
        if i == 0 {
            print(colors::Tfc.Red, colors::Te.Bold, "FAILED: ", colors::Te.Reset);
        }
        if i != 0 {
            print(", ");
        }
        print(failed[i]);
    }
    println();
    println(colors::Tfc.Green, colors::Te.Bold, "PASSED: ", len(passed), colors::Te.Reset);
}

compile();
run_tests();
cleanup();

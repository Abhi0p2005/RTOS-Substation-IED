# MASTER PROMPT: Substation IED Simulator (paste everything below into Claude Code CLI)

---

You are my senior embedded/firmware engineer and mentor. Build, step by step, a **Substation IED (protection relay) Simulator** on **macOS**. I have NOT built this before, so the repo must teach me as it grows. Work in phases and **stop after every phase** (see "Phase protocol").

## 0. Project goal and honesty rules

A deterministic protection-relay simulator that models what a Cortex-M + FreeRTOS IED does:
- 4 kHz sampling of 3-phase currents (80 samples per 50 Hz cycle)
- Sliding true-RMS (N = 80, O(1) per sample, running sum of squares, no sqrt in hot path)
- ANSI 50 (instantaneous overcurrent) and ANSI 51 (IEC 60255-151 IDMT inverse-time overcurrent, integrating accumulator for varying current, with reset characteristic)
- Latched trip output (simulated breaker pin)
- IEC 61850 GOOSE trip message: ASN.1 BER-encoded frame, Layer-2 EtherType 0x88B8, multicast 01-0C-CD-01-xx-xx, stNum/sqNum semantics, retransmission burst 0, 2, 4, 10 ms ... then heartbeat
- Sequence-of-Events (SOE) lock-free ring buffer with timestamps
- MMS/SCADA as a clearly labelled **stub/concept** only
- Task/priority model: Protection > GOOSE > SOE > MMS

**Honesty rules (enforce in code comments, README and docs):**
1. This is a **simulator and learning project**, NOT a certified IEC 61850 device. It implements a subset.
2. Every timing number produced on macOS is a **simulation result in virtual time**, not a hardware measurement. Label results "simulated" and never call design targets "measured".
3. Do not claim "zero jitter". Say sampling instants are timer-determined in the model and processing jitter is bounded by priorities.
4. Mark MMS and SV as "concept/stub" wherever they appear.

## 1. Environment (macOS)

Target: Apple Silicon or Intel Mac, macOS, Homebrew available.
Tools to use (create `scripts/check_env.sh` that verifies each and prints install hints):
- Required: Xcode Command Line Tools (clang, make), CMake, Git, Python 3 (numpy, matplotlib), Wireshark (tshark) for validating GOOSE pcaps
- Optional (Phase 8 only): `qemu-system-arm` and `arm-none-eabi-gcc` for a real Cortex-M3 + FreeRTOS build; Docker for the FreeRTOS POSIX port on Linux

Do **not** require root, raw sockets or BPF. GOOSE frames are written to a `.pcap` file (and optionally sent over loopback UDP as a demo) so everything runs without privileges. MMS stub listens on TCP port **10102** (not 102, which is privileged).

## 2. Architecture decisions (already made, do not ask me)

- Language: **C11** for the firmware-like core (portable, no heap, static allocation, no malloc after init). Host-side sim harness and tests also in C. Python only for tools (golden vectors, plots, stats).
- **Hardware abstraction layer (HAL)** with two backends behind one `hal.h`: `hal_sim` (macOS, virtual time) now; `hal_qemu` / real MCU later (optional).
- **Virtual-time, discrete-event simulation by default** so results are deterministic and reproducible on macOS. A 250 us sample tick advances the virtual clock; no dependence on wall-clock jitter. Optionally add a real-time pthread mode, clearly labelled non-deterministic on macOS.
- Core algorithms (`src/core`) must have **zero dependency on the HAL or the OS** so they can be dropped into real firmware unchanged.
- Model ADC + DMA ping-pong as a sim peripheral: fixed block size `B` (configurable, default 1, allowed 1..4), DMA-complete callback calls a tiny "ISR" that only notifies the protection "task". Model the added latency `B x 250 us`.
- Trip path writes the simulated trip pin immediately, then notifies the GOOSE task (never the other way round).

## 3. Repository layout to create

```
substation-ied-sim/
  README.md                  # full explanation, how to build/run/test, diagrams (ASCII/mermaid)
  ARCHITECTURE.md            # data flow, fault-to-trip timeline, task/priority table, design decisions
  CMakeLists.txt
  Makefile                   # thin wrapper: make build | test | run | pcap | report | clean
  .gitignore
  scripts/
    check_env.sh
    run_all_scenarios.sh
  src/
    config.h                 # Fs=4000, N=80, thresholds, curve selection, B, scaling
    core/
      rms.h rms.c            # sliding RMS (int32 window, int64 sumsq)
      idmt.h idmt.c          # SI/VI/EI/LTI curves; expm1f(alpha*logf(M)); optional LUT
      ansi50.h ansi50.c      # instantaneous element, optional 2-consecutive debounce
      ansi51.h ansi51.c      # IDMT accumulator + reset characteristic
      trip_fsm.h trip_fsm.c  # Idle/Armed/Tripped/Latched, reset command
      protection.h protection.c  # per-sample pipeline: offset removal -> RMS -> 50/51 -> trip
    hal/
      hal.h                  # timer, adc_dma, gpio_trip, time_now_us, eth_tx, log
      hal_sim.c              # virtual clock, ping-pong DMA model, sim trip pin with timestamped edges
    goose/
      ber.h ber.c            # TLV encode, short/long length form
      goose_pdu.h goose_pdu.c# gocbRef, timeAllowedtoLive, datSet, goID, t, stNum, sqNum, test, confRev, ndsCom, numDatSetEntries, allData
      goose_pub.h goose_pub.c# stNum/sqNum state machine, retransmission schedule 0/2/4/10 ms -> heartbeat
      pcap_writer.h pcap_writer.c
    soe/
      ring_buffer.h ring_buffer.c   # SPSC lock-free, documented drop policy
    mms_stub/
      mms_stub.c             # TCP 10102 stub, returns a canned status, labelled CONCEPT
    rtos_model/
      task_model.h task_model.c     # priority-ordered cooperative/preemptive model of the 4 tasks, WCET accounting
    sim/
      waveform.h waveform.c  # 3-phase 50 Hz generator + fault injection: step, ramp, marginal, DC offset, noise, harmonics, inrush-like spike, ADC saturation
      scenario.h scenario.c  # scenarios as small text/CSV files
      main_sim.c             # runs a scenario, writes CSV + pcap + event log
  scenarios/
    normal_load.scn
    severe_fault_10pu.scn
    fault_5pu.scn
    fault_3pu.scn
    marginal_2p05pu.scn
    overload_51_SI.scn
    inrush_spike.scn
    dc_offset.scn
  tests/
    test_rms.c test_idmt.c test_ansi50.c test_ansi51.c test_trip_fsm.c
    test_ber.c test_goose.c test_ring_buffer.c test_latency_model.c
    golden/                  # CSV vectors produced by tools/gen_golden.py
  tools/
    gen_golden.py            # Python reference for RMS and IDMT -> golden CSVs
    plot_results.py          # waveform, RMS, accumulator, trip instant plots (PNG)
    latency_stats.py         # min/mean/max/std/99.9th pct + histogram from many injected runs
    check_goose_pcap.sh      # tshark checks: EtherType 0x88B8, stNum/sqNum, retransmit spacing
  docs/
    TIMING_BUDGET.md         # budget table + how to measure on real HW (logic analyzer, DWT)
    VERIFICATION.md          # test plan, what was verified in sim vs what needs hardware
    GOOSE_FRAME.md           # byte-level frame layout with a worked hex example
    LEARNING_NOTES.md        # domain primer: IED, CT, ANSI 50/51, IDMT, GOOSE/MMS/SV, why L2
    INTERVIEW_NOTES.md       # honest implemented / designed / measured table + Q&A
    ROADMAP.md               # optional phases and future work
  firmware_port/             # created only in Phase 8 (optional QEMU Cortex-M3 + FreeRTOS)
```

## 4. Technical requirements per module (acceptance criteria)

- **RMS**: window N=80, `sumsq += x*x - old*old`, int64 accumulator, trip comparison on squared values: `sumsq > thr*thr*N`. Test against Python golden vectors; test DC offset, harmonics, saturation.
- **Detection latency**: reproduce this theory with the sim, using pre-fault 1 pu and 2 pu pickup: `k ~= 80*(T^2 - I0^2)/(F^2 - I0^2)`. Expected approx: F=10 pu -> ~3 samples (0.75 ms); 5 pu -> ~10 (2.5 ms); 3 pu -> ~30 (7.5 ms); 2.5 pu -> ~46 (11.5 ms); 2.05 pu -> ~75 (~18.8 ms). Show simulated vs theoretical in a table. Inception angle affects results, so report a range over angles.
- **IDMT**: `t = TD*beta/((I/Ip)^alpha - 1)`; SI 0.02/0.14, VI 1.0/13.5, EI 2.0/80, LTI 1.0/120. Unit tests must hit the worked examples at TD=0.5, I=5xIp: SI ~2.14 s, VI ~1.69 s, EI ~1.67 s. Accumulator `acc += Ts/t_op`, trip at acc >= 1, decay on drop below pickup. Use `expm1f(alpha*logf(M))` for precision near M=1.
- **Trip**: latched until explicit reset; records timestamp of the pin edge in virtual time.
- **GOOSE**: frame = dst MAC 01-0C-CD-01-xx-xx, src MAC, optional 802.1Q tag (PCP 4), EtherType 0x88B8, APPID/Length/Reserved1/Reserved2, then goosePdu (outer tag 0x61) with BER tags 0x80..0x8A and 0xAB allData. stNum++ and sqNum=0 on state change; sqNum++ on each retransmission; timeAllowedtoLive about 2x max retransmit interval. Output pcap must open in Wireshark/tshark and decode as GOOSE with correct counters and spacing (verify via `tools/check_goose_pcap.sh`). Pre-build a frame template and patch only stNum, sqNum, t and data.
- **SOE ring buffer**: single producer, single consumer, no locks, documented policy (drop-oldest or drop-newest), entries `{timestamp_us, event_id, value}`.
- **Task model**: priorities Protection > GOOSE > SOE > MMS; record per-task execution time in virtual cycles/us; report a WCET-style summary vs the 250 us sample period; show that a stalled MMS task cannot delay a trip (test it by injecting a long MMS stall).
- **Coding standards**: C11, `-Wall -Wextra -Werror`, no heap in `src/core` and `src/goose`, fixed-width ints, no VLAs, MISRA-flavoured style, `static_assert` on frame/struct sizes, run clang-tidy or cppcheck if available (do not require them).

## 5. Phase protocol (IMPORTANT)

Work through these phases in order. **After each phase: (1) build, (2) run that phase's tests, (3) update README.md and the relevant docs, (4) commit with a clear message, (5) print a short summary: what was built, how to run it, what I should learn from it, any caveats, and then STOP and wait for me to say "continue".** Do not start the next phase until I confirm. Do not ask me design questions; use the decisions above and state any assumption you make.

- **Phase 0: Scaffold.** Repo layout, CMake + Makefile, `check_env.sh`, `config.h`, README skeleton with the honesty section, `.gitignore`, first commit. Verify `make build` works on an empty main.
- **Phase 1: Signal source + HAL sim.** `waveform` generator (3-phase, per-unit scaling to 12-bit ADC codes, fault types), virtual clock, ADC/DMA ping-pong model with configurable B, tiny ISR -> notify model. Output CSV of samples. Plot with `plot_results.py`.
- **Phase 2: Sliding RMS.** `rms.c`, `gen_golden.py`, `test_rms.c`. Plot RMS vs time for a step fault.
- **Phase 3: ANSI 50, IDMT curves, ANSI 51.** Tests including the worked examples and varying-current accumulator behaviour.
- **Phase 4: Trip FSM + end-to-end protection pipeline + latency study.** Run N >= 1000 randomized fault-inception runs per fault magnitude; produce `latency_stats.py` output (min/mean/max/std/99.9th pct + histogram) and the simulated-vs-theory table. Write `docs/TIMING_BUDGET.md` with clear "simulated" labelling.
- **Phase 5: GOOSE.** BER encoder, PDU builder, publisher state machine, retransmission schedule, pcap writer, tshark validation script, `docs/GOOSE_FRAME.md` with annotated hex.
- **Phase 6: SOE + MMS stub + task/priority model.** Ring buffer tests, MMS stall-injection test proving trip timing is unaffected, WCET summary.
- **Phase 7: Scenarios + reports.** `run_all_scenarios.sh` runs every scenario and generates plots, CSVs, pcaps and a Markdown results report under `out/`. Finalize README (architecture, how to run, results, limitations).
- **Phase 8 (OPTIONAL, only if I say so): Real Cortex-M3 build.** FreeRTOS on QEMU `mps2-an385` with `arm-none-eabi-gcc`: SysTick, NVIC priority grouping, timer-driven sample interrupt feeding table-driven samples, `vTaskNotifyGiveFromISR` + `portYIELD_FROM_ISR`, static task allocation, task priorities as above, semihosting/UART output. Explain the `configMAX_SYSCALL_INTERRUPT_PRIORITY` rule in comments. Keep `src/core` shared and unchanged.
- **Phase 9: Docs finish.** `LEARNING_NOTES.md`, `VERIFICATION.md` (what is verified in simulation vs what needs real hardware: logic analyzer GPIO timing, DWT cycle counts, Wireshark on real Ethernet), `INTERVIEW_NOTES.md` (honest implemented/designed/measured table, 30 s and 2 min pitch, probable Q&A tied to this code), `ROADMAP.md` (hardware timestamping/PTP, SV 9-2LE, GOOSE subscriber closing the loop, PRP/HSR, IEC 62351, COMTRADE replay, 50N/51N, 67, 87, WCET analysis).

## 6. README requirements (grow it every phase)

The final README must let a beginner understand the whole project: what an IED is, the pipeline diagram, folder map with one line per file, build/run/test commands for macOS, how to read each output (CSV, PNG, pcap), a results section with simulated numbers clearly labelled, a "What is real vs simulated vs not implemented" table, and a link to each doc.

## 7. Start now

Begin with **Phase 0** only. Show me the file tree you created, the exact commands to build and test, and then wait for "continue".

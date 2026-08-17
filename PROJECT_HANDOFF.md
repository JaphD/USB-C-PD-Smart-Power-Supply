# PD Smart Supply — Layout Handoff

## Purpose and current stage

This is Rev A of a portfolio-oriented, USB-C PD-powered programmable supply. The board negotiates a USB-PD voltage, protects and monitors the selected output, powers an STM32 control/UI subsystem from 3.3 V, and exposes two parallel output connectors.

The research and schematic-design stage is complete. The next task should focus on board setup, BOM availability validation, component placement, routing, planes, and PCB DRC. Do not reconsider confirmed circuit decisions unless inspection finds concrete contradictory evidence.

Latest completed schematic checkpoint:

- Commit: `21968eb Complete Rev A schematic with sensing and test points`
- Main files: `PD Smart Supply.kicad_sch`, `PD Smart Supply.kicad_pcb`, and `PD Smart Supply.kicad_pro`
- Supporting references: `Docs/Ref/` is the authoritative local datasheet collection. It includes the CH224A, OLED, TPS25947, TPS560430, and STM32G071 references. Consult these files first when a layout decision depends on a manufacturer's pinout, land pattern, thermal guidance, or placement recommendation.
- `Sim/` contains the validated EN/UVLO MCU-control simulation. The contents of `Tool/` are not part of the layout handoff and do not need to be consulted.

## System architecture

Power and control flow:

`USB-C input -> VBUS protection -> CH224A PD negotiation -> VBUS_PD -> TPS259470L eFuse -> EFUSE_OUT -> two parallel output connectors`

`VBUS_PD -> TPS560430 buck converter -> +3.3V -> STM32G071CBT6, OLED, encoder, status circuitry`

The intended external output rating is up to 20 V at 3 A, approximately 60 W. The two output connectors share the same protected output and ground; their combined load is 3 A, not 3 A per connector.

## USB-C and CH224A decisions

- USB-C receptacle: USB4105-GF-A.
- CH224A is used in three-GPIO voltage-selection mode.
- Supported selections for this design are 5 V, 9 V, 12 V, and 20 V.
- The current CH224A GPIO table does not provide 15 V. Its `0,1,0` state requests 28 V, so firmware must never generate that combination.
- The 28 V mode is intentionally disabled because the design, including the TVS2200, is limited to 20 V operation.
- USB D+ mapping was corrected: connector A6/B6 -> CH224A DP.
- USB D- mapping was corrected: connector A7/B7 -> CH224A DM.
- CC1 and CC2 connect directly to the matching CH224A pins.
- CH224A VHV and VBUS connect to `VBUS_PD`; VHV has 1 uF to GND.
- `CFG1` has a 10 kOhm pull-up to 3.3 V, giving a safe default 5 V request.
- `PG` is open-drain and has a 10 kOhm pull-up to 3.3 V.
- TVS2200DRVR is already fitted on `VBUS_PD`; do not add a duplicate VBUS TVS at the eFuse.
- Keep DP/DM short, paired, without test pads or branches, and referenced continuously to the In1 GND plane.

CH224A MCU nets:

| Function | MCU pin/net |
|---|---|
| CFG1 | PC6 |
| CFG2 | PC7 |
| CFG3 | PA8 |
| PD PG | PA15 / `PG` |

## TPS259470L eFuse decisions

- Part: `TPS259470LRPWR`.
- IN connects to `VBUS_PD`; OUT is `EFUSE_OUT`.
- Input capacitor: 1 uF ceramic close to IN and GND.
- Output capacitor: 10 uF ceramic close to OUT and GND. A separate 100 nF output capacitor was judged unnecessary.
- Negative-output clamp: SS54 with cathode to `EFUSE_OUT` and anode to GND. Place beside the eFuse OUT pin and output capacitor with a very small loop.
- `ITIMER` is intentionally left open for the minimum overcurrent-response delay.
- `dVdt`: 2.2 nF C0G, rated 50 V, from dVdt to GND.
- The calculated slew rate is approximately 0.91 V/ms, giving approximately 22 ms rise time at 20 V.
- `RILM`: 887 Ohm, 1%, from ILM to GND. This is intentional and must not be casually changed.
- The nominal current-limit threshold is approximately `3334 / 887 = 3.76 A`. The headroom was selected so component and IC tolerances do not nuisance-limit a legitimate 3 A load. The user-facing output rating remains 3 A.
- ILM is also the analog current-monitor output. With typical gain 182 uA/A and 887 Ohm, its nominal scaling is about 0.1614 V/A: 1 A -> 0.161 V, 2 A -> 0.323 V, 3 A -> 0.484 V.
- ILM is capacitance-sensitive. Keep RILM beside the IC, keep the `IOUT_SENSE` trace and test-point stub short, and keep total loading below the datasheet's approximately 50 pF limit. Do not place a capacitor directly from ILM to GND.
- `FLT` and `AUXOFF` each use 10 kOhm pull-ups to 3.3 V.
- `EFUSE_READY` is AUXOFF. It indicates a valid input and completed inrush, but not every load-side fault. Use `EFUSE_nFLT` for fault reporting.

OVLO divider:

- Upper path: 820 kOhm + 49.9 kOhm in series from `VBUS_PD`.
- Lower resistor: 49.9 kOhm from OVLO to GND.
- Typical threshold using the 1.20 V comparator threshold is approximately 22.12 V.
- Place the lower resistor and OVLO junction close to the OVLO pin.

EN/UVLO MCU shutdown:

- 2.2 MOhm from `VBUS_PD` to EN/UVLO.
- 2N7002 drain to EN/UVLO and source to GND.
- Gate driven through 1 kOhm by `EFUSE_OFF`.
- 100 kOhm gate pull-up to 3.3 V creates a fail-off default during MCU reset.
- `EFUSE_OFF = HIGH` disables the eFuse; LOW enables it.
- This circuit was validated in LTspice; simulation files are under `Sim/`.

eFuse MCU nets:

| Function | MCU pin/net |
|---|---|
| Voltage sense | PA0 / `VOUT_SENSE` |
| Current sense | PA1 / `IOUT_SENSE` |
| eFuse fault | PB15 / `EFUSE_nFLT` |
| eFuse ready/AUXOFF | PB14 / `EFUSE_READY` |
| eFuse shutdown | PB13 / `EFUSE_OFF` |

## Output-voltage sensing

- Divider upper resistance: 49.9 kOhm + 49.9 kOhm in series from `EFUSE_OUT`.
- Divider lower resistance: 10 kOhm to GND.
- 100 nF from `VOUT_SENSE` to GND.
- ADC input: STM32 PA0.
- `VADC = VOUT * 10k / 109.8k`.
- At 20 V, VADC is approximately 1.82 V.
- Firmware conversion factor is approximately 10.98.
- The filter settles on the millisecond scale; wait approximately 5 ms after a voltage transition before treating the reading as stable.
- Route this net away from the buck SW node and inductor.

## 3.3 V buck regulator

- Regulator: TPS560430-based 3.3 V rail.
- Input: `VBUS_PD`.
- Input capacitors: two 2.2 uF plus 100 nF.
- Bootstrap capacitor: 100 nF between CB and SW.
- Inductor: 10 uH.
- Output capacitor: 22 uF.
- Feedback divider: 51 kOhm upper and 22.1 kOhm lower.
- The feedback divider must sense after the inductor at the regulated 3.3 V output.
- Keep the hot loop compact. Keep the SW-to-inductor connection short and its copper area small; do not route sensitive signals beneath or adjacent to it.

## MCU and interface assignments

MCU: STM32G071CBT6.

| Function | MCU pin |
|---|---|
| VOUT sense | PA0 |
| IOUT sense | PA1 |
| Encoder A | PA6 |
| Encoder B | PA7 |
| Encoder switch | PB0 |
| OLED SCL | PB6 |
| OLED SDA | PB7 |
| CH224A CFG1 | PC6 |
| CH224A CFG2 | PC7 |
| CH224A CFG3 | PA8 |
| CH224A PG | PA15 |
| SWDIO | PA13 |
| SWCLK | PA14 |
| eFuse nFLT | PB15 |
| eFuse ready | PB14 |
| eFuse off | PB13 |

MCU supply/reset decisions:

- 3.3 V decoupling includes 4.7 uF bulk and local 100 nF capacitors.
- NRST uses a 10 kOhm pull-up, 100 nF to GND, and a pushbutton to GND.
- SWD header intentionally remains four pins: SWDIO, SWCLK, NRST, and GND.
- No VTref pin is included because the proven debugger uses fixed 3.3 V signaling and worked on the user's earlier board. Do not change this without a specific debugger requirement.

OLED:

- Four pins: GND, VCC, SCL, SDA.
- Powered from 3.3 V with 100 nF local bypass.
- SCL and SDA each have 4.7 kOhm pull-ups to 3.3 V.

Rotary encoder:

- Part source/datasheet reference: JLC/LCSC `C209762`; footprint and pin mapping were reviewed and confirmed.
- A, B, and switch inputs each use 1 kOhm pull-ups to 3.3 V.
- Common encoder and switch terminals connect to GND.
- Mechanical debouncing and quadrature decoding are handled in firmware.

Error LED:

- CH224A `PG` drives a 2N7002 gate.
- Source to GND; drain sinks the LED through a 1 kOhm series resistor from 3.3 V.
- The transistor intentionally inverts PG: LED on means PD not ready/fault; LED off means power good.

## Test points

Use bare, top-side circular SMD pads with no fitted component:

- Symbol: `Connector:TestPoint`
- Footprint: `TestPoint:TestPoint_Pad_D1.5mm`
- Nets: `VBUS_PD`, `+3.3V`, `EFUSE_OUT`, `VOUT_SENSE`, `IOUT_SENSE`, and GND.
- Label the `EFUSE_OUT` test point as `VOUT` on silkscreen; `VOUT` is the user-facing name for the protected `EFUSE_OUT` net.
- Keep the `IOUT_SENSE` pad immediately beside RILM with no long stub.
- Keep approximately 1 mm free space around pads when practical, and put labels beside rather than under them.
- Do not add USB DP/DM or CC test pads.

## PCB manufacturing target and stackup

Manufacturer: JLCPCB. Board target: four layers, nominal 1.6 mm, FR-4, 1 oz outer copper, 0.5 oz inner copper, green solder mask, white silkscreen, and HASL with lead.

Selected stackup: `JLC04161H-7628`.

Enter this physical stack from top to bottom:

| Layer | Material/type | Thickness | Relative permittivity |
|---|---|---:|---:|
| F.Mask | Solder mask | 0.010 mm | 3.8 |
| F.Cu | Copper, 1 oz | 0.035 mm | — |
| Dielectric 1 | FR-4 7628 prepreg | 0.2104 mm | 4.4 |
| In1.Cu | Copper, 0.5 oz | 0.0152 mm | — |
| Dielectric 2 | FR-4 core | 1.065 mm | 4.6 |
| In2.Cu | Copper, 0.5 oz | 0.0152 mm | — |
| Dielectric 3 | FR-4 7628 prepreg | 0.2104 mm | 4.4 |
| B.Cu | Copper, 1 oz | 0.035 mm | — |
| B.Mask | Solder mask | 0.010 mm | 3.8 |

KiCad shows approximately 1.6062 mm from these explicit values; this is expected. Do not use `Adjust Dielectric Thicknesses`. Enable `Impedance controlled` in the Physical Stackup dialog.

Layer functions:

- F.Cu: components, critical signals, USB pair, and high-current routing/pours.
- In1.Cu: uninterrupted GND plane.
- In2.Cu: 3.3 V/power regions and limited secondary routing where necessary.
- B.Cu: secondary signals and GND fill.

Board-finish/setup recommendations not yet fully walked through:

- HASL with lead, no edge connector, no castellations, no edge plating.
- Leaded HASL is accepted for this Rev-A prototype because it is economical, durable, reworkable, and supported by JLCPCB. Its surface is less planar than ENIG, so verify the 0.45 mm-pitch TPS259470 QFN land pattern and exposed-pad paste apertures carefully in JLCPCB's assembly viewer. This choice is not RoHS compliant, and JLCPCB supports only its high-temperature solder-paste process with leaded HASL.
- Tent ordinary vias on both sides.
- Global mask and paste clearances at zero so footprints control their openings.
- Minimum solder-mask web target: 0.10 mm.
- Do not enable solder-mask bridges within footprints globally.

## Planned net classes

These were selected conceptually but still need to be entered and validated in PCB Editor:

| Class | Nets | Width | Clearance | Via size/drill |
|---|---|---:|---:|---:|
| `POWER_3A` | `VBUS_PD`, `EFUSE_OUT` | 1.5 mm minimum; prefer wider pours | 0.25 mm | 0.8/0.4 mm |
| `POWER_3V3` | `+3.3V` | 0.50 mm | 0.20 mm | 0.6/0.3 mm |
| `BUCK_SW` | TPS560430 SW net | 0.80 mm, short and compact | 0.25 mm | Avoid vias |
| `USB2_DIFF` | `USB_P`, `USB_N` | Stackup-calculated | Stackup-calculated | Avoid vias |
| `ANALOG_SENSE` | `VOUT_SENSE`, `IOUT_SENSE` | 0.20 mm | 0.20 mm | 0.6/0.3 mm |
| `SIGNAL` | Remaining signals | 0.20 mm | 0.20 mm | 0.6/0.3 mm |

For 3 A nets, prefer 2–3 mm copper widths or pours where space permits. If a high-current net must change layers, use at least two or three 0.4 mm finished-hole vias in parallel. Keep the GND net as planes/pours rather than treating it as a narrow routed trace.

USB differential width and gap are not yet finalized. Calculate them against the exact JLC04161H-7628 stackup using JLCPCB's impedance calculator. Do not assume the temporary 0.20 mm width/gap values are the final 90 Ohm geometry.

## Layout-critical placement priorities

Recommended placement order:

1. Board outline, mounting holes, USB-C connector, encoder, OLED connection, output connectors, SWD, the MCU's side-access reset button, and other mechanically constrained parts. Position the reset button on the chosen enclosure-accessible board edge before placing the MCU block, and reserve finger/tool access around its actuator.
2. VBUS TVS and USB/CC ESD protection immediately beside the USB-C connector.
3. CH224A close to the connector with direct CC and short paired DP/DM routing.
4. TPS259470, input capacitor, output capacitor, SS54, OVLO/RILM/dVdt parts, then output connectors.
5. TPS560430 input capacitors, IC, bootstrap capacitor, inductor, output capacitor, and feedback divider as one tight power block.
6. STM32, its decoupling, reset, SWD, and ADC filtering.
7. OLED, encoder, LED, and test points.

Maintain an obvious high-current flow:

`USB-C -> TVS/VBUS_PD -> eFuse IN -> eFuse OUT -> output connectors`

Keep the CH224A USB region and both ADC-sense traces away from the buck SW node and inductor. Place every decoupling capacitor beside its target pin with a short return to the In1 GND plane. Stitch top/bottom GND copper to In1 near the USB connector, TVS, eFuse, buck converter, MCU, and output connectors.

## Outstanding work for the new layout chat

1. Inspect Git status and the actual `.kicad_pcb` state; the stackup dialog may contain uncommitted changes made after commit `21968eb`.
2. Confirm the physical stackup values and enable impedance control.
3. Complete Board Finish, Solder Mask/Paste, Constraints, Pre-defined Sizes, Net Classes, and any custom differential-pair rules.
4. Run schematic annotation/ERC if not already run and resolve genuine errors.
5. Verify every exact symbol-to-footprint pin mapping, especially USB-C, CH224A exposed pad, TPS259470 QFN, TPS560430, rotary encoder, SS54 polarity, and OLED connector order.
6. Generate the BOM now and run it through the user's stock-availability tracker before placement. Regenerate it later for fabrication.
7. Update PCB from schematic.
8. Place mechanical items and functional blocks in the order above.
9. Request a placement review before routing.
10. Route USB and switching/power-critical loops first, then power, analog sensing, and ordinary signals.
11. Add planes/pours and stitching vias, run DRC, inspect Gerbers/assembly outputs, and prepare JLCPCB fabrication/BOM/CPL files.

## New-chat starter prompt

Use this in the new Codex chat:

> Read `PROJECT_HANDOFF.md`, inspect the latest schematic, PCB, project settings, Git status, and commit history, then continue the PCB-layout stage from the Outstanding work section. Treat the documented circuit decisions as confirmed unless you find concrete contradictory evidence. The immediate task is to finish the JLCPCB-tailored Board Setup and net classes before component placement.

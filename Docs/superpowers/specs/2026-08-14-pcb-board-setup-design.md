# PD Smart Supply Rev A — PCB Board Setup Design

## Scope

Configure KiCad 9 Board Setup for the existing Rev A schematic before placement or routing. This specification covers the physical stackup, layer functions, fabrication finish, global constraints, predefined routing sizes, net classes, and special routing rules. It does not change the schematic, component selection, board outline, placement, or routing.

## Manufacturing target

- Fabricator: JLCPCB
- Layer count: 4
- Stackup: JLC04161H-7628
- Nominal board thickness: 1.6 mm; KiCad calculated thickness 1.6062 mm is accepted
- Outer copper: 0.035 mm (1 oz)
- Inner copper: 0.0152 mm (0.5 oz)
- Solder mask: green
- Silkscreen: white
- Surface finish: HASL with lead
- Edge connector, castellations, and edge plating: none
- Ordinary through vias: tented on front and back

## Physical stackup

| Order | KiCad layer/material | Type | Thickness | Relative permittivity | Function |
|---:|---|---|---:|---:|---|
| 1 | F.Mask | Solder mask | 0.0100 mm | 3.8 | Top solder mask |
| 2 | F.Cu | Copper | 0.0350 mm | — | Components, critical signals, high-current pours, USB pair |
| 3 | Dielectric 1, FR-4 7628 | Prepreg | 0.2104 mm | 4.4 | Top-to-In1 dielectric |
| 4 | In1.Cu | Copper | 0.0152 mm | — | Uninterrupted GND plane |
| 5 | Dielectric 2, FR-4 | Core | 1.0650 mm | 4.6 | Inner core |
| 6 | In2.Cu | Copper | 0.0152 mm | — | Uninterrupted GND plane |
| 7 | Dielectric 3, FR-4 7628 | Prepreg | 0.2104 mm | 4.4 | In2-to-bottom dielectric |
| 8 | B.Cu | Copper | 0.0350 mm | — | Secondary signals and local power routing/pours |
| 9 | B.Mask | Solder mask | 0.0100 mm | 3.8 | Bottom solder mask |

Enable KiCad's impedance-controlled/dielectric-constraints setting. Do not use **Adjust Dielectric Thicknesses**.

Both inner copper layers remain GND. They must not contain signal traces or power islands. F.Cu and B.Cu may contain GND fills, joined to both internal planes with stitching vias. The second GND layer gives B.Cu a continuous reference and makes top-to-bottom signal transitions use an adjacent GND stitching via without requiring a reference-transfer capacitor.

## Board finish and mask/paste

- Copper finish: HASL with lead
- Global solder-mask-to-copper clearance: 0
- Global solder-paste clearance and ratio: 0
- Allow solder-mask bridges inside footprints: disabled
- Minimum intended solder-mask web: 0.10 mm
- Tent ordinary vias on both sides; explicitly untent only test points or other intentionally exposed copper
- Footprints control local mask and paste geometry, especially the TPS259470 exposed-pad aperture

## Global constraints

The design rules are conservative project constraints rather than an attempt to encode JLCPCB's absolute process limits.

| Constraint | Value |
|---|---:|
| Minimum copper clearance | 0.20 mm |
| Minimum track width | 0.20 mm |
| Minimum through-via diameter | 0.60 mm |
| Minimum through-via drill | 0.30 mm |
| Minimum via annular width | 0.15 mm |
| Minimum through-hole diameter | 0.30 mm |
| Minimum copper-to-board-edge clearance | 0.50 mm |
| Minimum hole-to-hole clearance | 0.25 mm |
| Minimum copper-to-hole clearance | 0.25 mm |
| Minimum silkscreen text height | 0.80 mm |
| Minimum silkscreen text thickness | 0.10 mm |

Microvias are not used and should not be selected for this board.

## Predefined routing sizes

Add these track widths to KiCad's predefined list:

- 0.20 mm — ordinary and analog signals
- 0.25 mm — optional robust signal routing
- 0.50 mm — 3.3 V distribution
- 0.80 mm — compact buck SW connection
- 1.50 mm — minimum 3 A power route
- 2.00 mm and 3.00 mm — preferred high-current routes where pours are impractical

Add these through-via sizes:

- 0.60/0.30 mm diameter/drill — ordinary signal and 3.3 V transitions
- 0.80/0.40 mm diameter/drill — high-current and ground stitching

High-current layer changes use at least three 0.80/0.40 mm vias in parallel unless a later current/thermal calculation demonstrates that fewer are adequate. The preferred solution is to keep the 3 A path on F.Cu as wide copper pours and avoid layer changes.

## Net classes

KiCad net names currently include a leading `/` for locally labelled nets; assignments must use the actual PCB net names.

| Class | Assigned nets | Width | Clearance | Via diameter/drill | Differential width/gap |
|---|---|---:|---:|---:|---:|
| `Default` | Unassigned ordinary signals | 0.20 mm | 0.20 mm | 0.60/0.30 mm | — |
| `POWER_3A` | `/VBUS_PD`, `/EFUSE_OUT` | 1.50 mm minimum | 0.25 mm | 0.80/0.40 mm | — |
| `POWER_3V3` | `+3.3V` | 0.50 mm | 0.20 mm | 0.60/0.30 mm | — |
| `BUCK_SW` | `Net-(U2-SW)` | 0.80 mm | 0.25 mm | 0.60/0.30 mm, but vias prohibited by rule | — |
| `USB2_DIFF` | `/USB_P`, `/USB_N` | Calculated value | 0.20 mm | 0.60/0.30 mm, but avoid vias | Calculated for 90 Ω differential |
| `ANALOG_SENSE` | `/VOUT_SENSE`, `/IOUT_SENSE` | 0.20 mm | 0.20 mm | 0.60/0.30 mm | — |

The USB width and gap must be calculated for JLC04161H-7628 using JLCPCB's controlled-impedance calculator before routing. Temporary values must not be presented as final impedance geometry. Because the pair is short and used by the CH224A rather than a general USB data interface, routing it short, paired, and continuously referenced to In1 is more important than length tuning.

## Custom-rule intent

Create a project-local `.kicad_dru` file where net-class settings alone cannot express the requirement:

1. Prohibit vias on `Net-(U2-SW)`.
2. Restrict `/USB_P` and `/USB_N` to F.Cu unless an unavoidable placement constraint is approved later.
3. Apply the finalized differential-pair width and gap to `/USB_P` and `/USB_N`.
4. Do not use a blanket rule to prohibit routing on In1/In2; those layers are reserved operationally for GND zones, and the completed layout is verified for violations during review and DRC.

## Placement and routing consequences

- The TPS560430, input capacitors, bootstrap capacitor, inductor, output capacitor, and feedback network form a compact top-side block.
- In1 remains solid beneath the buck block except where component through-holes make unavoidable antipads. No deliberate cutout is placed beneath the SW node; the SW copper itself remains short and compact on F.Cu.
- USB D+/D− remain on F.Cu over uninterrupted In1 GND, with no test pads, branches, or layer changes.
- `IOUT_SENSE` and its test pad remain extremely short at RILM; neither is routed near the buck SW node or inductor.
- `VOUT_SENSE` is routed away from the buck and may be guarded by ordinary GND copper where useful.
- `VBUS_PD` and `EFUSE_OUT` use wide F.Cu pours following the physical power flow. They do not consume an inner plane.
- 3.3 V uses a combination of 0.50 mm routing and local outer-layer pours. Every IC receives local decoupling with a short GND via into In1.
- Add GND stitching near the USB shield/ESD region, TVS, eFuse, buck, MCU, output connectors, board perimeter where useful, and beside signal vias that change between F.Cu and B.Cu.

## Verification

Board Setup is complete when:

1. The saved PCB reports the explicit 1.6062 mm stackup and both inner layers are assigned as GND planes by design intent.
2. Copper finish, mask/paste, via tenting, constraints, predefined sizes, and net-class assignments match this document.
3. The project-local custom rules parse successfully in KiCad.
4. KiCad DRC reports no rule-configuration or malformed-rule errors.
5. The actual USB differential geometry has been verified with JLCPCB's calculator before USB routing begins.
6. No schematic, footprint placement, board outline, or routing is unintentionally changed during Board Setup configuration.

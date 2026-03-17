# Dexterous Hand Control System for Precision Grasping

A robotics project focused on **precision grasping**, **embedded control**, and **hardware-software integration** using an Arduino-based dexterous hand prototype.

This project presents a dexterous hand control system designed to improve grasp stability for objects of different sizes through a staged control strategy. The system combines a **pinch-first grasping mechanism**, **position-synchronized follower fingers**, and a **fine adjustment mode** for small-object manipulation.

The thumb and index finger are driven by **LV-TTL motors** to perform the primary pinch action, while the remaining three fingers are driven by **PWM servos** to provide follow-up wrapping support. To improve reliability and reduce latency during prototyping, a **button-based control interface** was adopted instead of Bluetooth communication.

My main contributions include **control architecture design**, **Arduino firmware implementation**, **parameter tuning**, **hardware-software integration**, and **grasp performance testing**.

---

## Project Highlights

- **Pinch-first grasping strategy** for initial object capture
- **Load-threshold stop logic** for safer and more stable contact
- **Position-synchronized follower fingers** for coordinated wrapping
- **Fine adjustment mode** for small-object grasping
- **Low-latency button-based control** for reliable prototyping
- **Integrated embedded control and robotic hardware implementation**

---

## Motivation

Dexterous robotic grasping requires both precise initial contact and stable follow-up wrapping. In low-cost prototypes, simple open-loop control often leads to unstable grasps, delayed response, or poor performance on small targets.

This project explores a lightweight and practical control solution that balances:

1. **stable initial pinch contact**,
2. **coordinated finger wrapping**,
3. **limited fine adjustment for small or difficult objects**.

The goal was not only to make the hand move, but to design a control strategy that is **interpretable**, **tunable**, and **effective in real hardware tests**.

---

## My Contributions

I was primarily responsible for the following aspects of the project:

- designed the overall control architecture of the dexterous hand,
- implemented the Arduino-based control firmware,
- integrated LV-TTL motor control and PWM servo coordination,
- designed the button-based interaction logic for grasp control,
- tuned parameters such as motion limits, load thresholds, and fine adjustment step size,
- conducted grasp testing and organized project documentation, videos, and results.

---

## System Overview

The system is built around an **Arduino controller** and consists of two main actuation groups:

- **Pinch fingers:** thumb and index finger driven by LV-TTL motors for initial object capture
- **Follower fingers:** three PWM-driven fingers for synchronized wrapping after pinch contact

The system uses a **button-based interface** to trigger grasping, opening, and fine adjustment actions. This design was chosen to reduce control latency and improve repeatability during hardware testing.

### Hardware Components

- Arduino controller
- 2 × LV-TTL motors
- 3 × PWM servos
- push buttons for user input
- power supply and wiring interface

---

## Control Strategy

The control logic consists of four main stages.

### 1. Open State

The hand remains open and ready for a new grasp.

### 2. Pinch Phase

The thumb and index finger move toward each other until contact is detected through a **load-threshold condition**.

### 3. Wrap Phase

After pinch contact, the remaining three fingers move according to a **position-synchronized mapping strategy**, improving overall grasp stability.

### 4. Fine Adjustment Phase

For small or difficult objects, an additional **fine adjustment step** is used to slightly refine finger position and improve grasp success.

This staged strategy separates **initial capture** from **grasp stabilization**, making the system easier to tune and easier to analyze during testing.

---

## Design Trade-off: Buttons vs Bluetooth

Although Bluetooth control offers greater flexibility, it introduced additional latency and reduced repeatability during early-stage hardware testing. Since this project focused on **grasp reliability** and **rapid prototyping**, a button-based interface was chosen as a more stable and lower-latency solution.

This reflects an important engineering trade-off:

- sacrificing some communication convenience,
- in order to improve robustness and repeatability during prototype validation.

---

## Wiring and Pin Map

Please update this section according to your final hardware configuration.

| Module | Connection |
|---|---|
| LV-TTL Motor 1 | Thumb / pinch finger |
| LV-TTL Motor 2 | Index / pinch finger |
| PWM Servo 1 | Follower finger 1 |
| PWM Servo 2 | Follower finger 2 |
| PWM Servo 3 | Follower finger 3 |
| Button A | Grasp trigger |
| Button B | Open / release |
| Button C | Fine adjustment |

Detailed wiring notes can be found in the `hardware/` and `docs/` folders.

---

## Key Parameters

The system behavior is mainly affected by the following parameters:

- **motion limits** for pinch fingers
- **load threshold** for contact detection
- **position mapping** between pinch motor and follower fingers
- **fine adjustment step size**
- **initial open position calibration**

These parameters were tuned empirically through repeated hardware testing to balance **grasp stability**, **responsiveness**, and **safe movement range**.

See `docs/calibration.md` for calibration notes.

---

## Experimental Results

The prototype was tested on objects of different sizes and grasping difficulty.

### Main Observations

- medium and large objects could be grasped with high stability,
- small objects were more sensitive to alignment and finger position,
- the fine adjustment mode improved grasp feasibility for small targets,
- synchronized follower fingers improved wrapping support after the initial pinch.

### Example Result Summary

| Object Type | Grasp Mode | Result |
|---|---|---|
| Medium cylindrical object | Pinch + Sync | Stable grasp |
| Large object | Pinch + Sync | Stable grasp |
| Small object | Pinch + Sync + Fine Adjust | Partially successful |
| Thin / difficult object | Pinch + Sync + Fine Adjust | Challenging |

More detailed logs can be found in the `results/` folder.

---

## Suggested Future Experimental Upgrade

To strengthen this project as a research-oriented portfolio piece, future experiments can include:

- **grasp success rate comparison** across object categories,
- **ablation study**:
  - pinch only,
  - pinch + sync,
  - pinch + sync + fine adjust,
- **response time measurement** for contact stop and adjustment,
- **failure case analysis** for small, thin, and low-friction objects.

---

## Limitations and Future Work

While the current prototype demonstrates stable grasping for medium and large objects, several limitations remain:

- small, thin, or low-friction objects are still difficult to grasp reliably,
- the current system relies on threshold-based contact handling rather than richer tactile sensing,
- follower fingers improve grasp wrapping but do not yet adapt independently to object geometry,
- the overall control strategy is still relatively simple and does not include full closed-loop grasp optimization.

Possible future improvements include:

- adding tactile or force sensing,
- improving closed-loop coordination across all fingers,
- introducing object-specific grasp planning,
- comparing multiple control strategies through systematic experiments.

---

## Repository Structure

```text
dexterous-hand-md20/
├── docs/           # design notes, calibration, diagrams
├── firmware/       # Arduino control code
├── hardware/       # wiring, BOM, hardware-related files
├── media/          # demo videos and images
├── results/        # grasp logs, summaries, test outputs
└── README.md
```

## How to Run

1. connect the motors, servos, and buttons according to the wiring guide,
2. upload the Arduino firmware to the Arduino controller,
3. power the system and verify the initial open position,
4. test pinch, wrap, and fine adjustment functions,
5. tune the key parameters if necessary.

For more details, see the calibration and hardware notes in `docs/`.

---

## Media

Demo videos and images are available in the `media/` folder.

Recommended demonstration content:

- full system overview,
- basic grasp sequence,
- synchronized finger wrapping,
- fine adjustment on a small object.

---

## Research Relevance

This project reflects my interest in robotic manipulation, embedded control, and hardware-software integration. Through this work, I developed hands-on experience in designing a robotic control system, implementing embedded logic, tuning hardware behavior, and evaluating grasp performance.

It also motivated me to further explore advanced research topics such as:

- tactile feedback,
- adaptive grasping,
- closed-loop robotic manipulation,
- embodied intelligence in robotic hands.

---

## Author

Kaiyang Deng  
Robotics Engineering, South China University of Technology  
GitHub: KaiSANG121

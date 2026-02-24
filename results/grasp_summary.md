# Grasp & Manipulation Benchmark Summary

This project is evaluated on two levels:
1) **Grasping** (pick & hold objects)
2) **Manipulation** (perform tasks beyond grasping, requiring alignment / insertion / torsion)

---

## 1. Overall Results

### Grasping (object pick & hold)
- **Medium / large rigid objects** (ping-pong ball, Rubik’s cube, half-full water bottle): typically **high success**  
  Key factor: **pinch + full-hand wrap** provides stable normal force and friction.
- **Compliant / deformable objects** (nylon rope, bagged milk, glasses cloth): typically **high success**  
  Key factor: **wrap** reduces slip and stabilizes contact on deformable surfaces.
- **Small objects** (thumbtack): ~**30%** success  
  Primary bottleneck: **pose accuracy** and **contact geometry sensitivity**.

### Manipulation (task completion)
- **Card pick & insertion**: requires **alignment + fine adjustment**  
- **Loosening bottle cap**: requires **torsion + anti-slip stability**, wrap helps maintain grip

> Detailed logs are in `results/grasp_tests.csv`.

---

## 2. Why the design works (mechanism-level explanation)

### (A) Load-threshold pinch (thumb + index, LV-TTL)
During GRIP, each LV-TTL motor closes until `load > kLoadThreshold`, then stops at current pose.
- Prevents excessive squeeze and reduces stall risk
- Provides a consistent “contact detected” event for higher-level behaviors

### (B) Full-hand wrap via SYNC (3× PWM follower fingers)
After pinch contact, pressing SYNC maps **index Motor2 position ratio** to PWM pulses and wraps the object.
- Increases total contact area
- Increases friction and stability
- Especially helpful for deformable objects (rope, fabric, bagged items) and heavier payloads

### (C) Fine adjustment
For small objects, coarse motion is fast but insufficient.
Fine IN/OUT buttons allow incremental pose corrections to improve alignment and contact quality.

---

## 3. Failure Modes (observed)
### Small object grasp (thumbtack)
Common failures:
- slip during initial pinch contact
- misalignment causing ejection
- insufficient normal force at the exact contact geometry

---

## 4. Future Improvements
- Reduce `kFineStep` (e.g., 20) to improve precision for small objects
- Add a “micro-search” routine: approach → detect contact → small local search around pose
- Add sensing on follower fingers (tactile / force / current) to close the loop
- Improve fingertip material (higher friction pads) for small rigid objects
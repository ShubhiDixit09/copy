# Chapter 6: Deterministic Core Algorithms

## 6.1 Possession Continuity Index (PCI)
The foundational mathematical metric of DHARTI is **verified continuous constructible frontage**, not gross area acquired.

### Mathematical Formulation
Let the total corridor alignment have length $L_{\text{total}}$.  
Intersecting the corridor centerline with cadastral parcel boundaries yields ordered chainage intervals:
$$I = \{[s_0, e_0], [s_1, e_1], \dots, [s_{n-1}, e_{n-1}]\}$$

For each interval $k$, an interval is **evidence-ready** if and only if:
$$\text{ready}(k) = (\text{ParcelState}_k = \text{ConstructionReady}) \land (\text{PaymentState}_k = \text{ReceiptConfirmed}) \land (\text{ActiveStay}_k = \text{False})$$

Merging all contiguous ready intervals $[s_i, e_i]$ yields continuous constructible segments:
$$\mathcal{S}_{\text{ready}} = \bigcup_{k \in \text{Ready}} [s_k, e_k]$$

The longest continuous frontage $L_{\text{continuous}}^{\max}$ is:
$$L_{\text{continuous}}^{\max} = \max_{j} (\text{length}(S_j)) \quad \text{where } S_j \in \mathcal{S}_{\text{ready}}$$

The **Possession Continuity Index (PCI)** is:
$$\text{PCI} = \frac{L_{\text{continuous}}^{\max}}{L_{\text{total}}}$$

### 6.1.1 Unlock Simulation Algorithm
To guide District Collectors and acquisition authorities where to focus immediate resources:
1. Identify all blocked parcels $\mathcal{P}_{\text{blocked}} = \{p \mid \text{ready}(p) = \text{False}\}$.
2. For each parcel $p \in \mathcal{P}_{\text{blocked}}$:
   - Simulate flipping $\text{ready}(p) = \text{True}$.
   - Recompute the longest continuous run $L_{\text{simulated}}^{\max}(p)$.
   - Calculate unlock gain: $\Delta L(p) = L_{\text{simulated}}^{\max}(p) - L_{\text{continuous}}^{\max}$.
3. Rank all blocked parcels in descending order of $\Delta L(p)$. The parcel with highest $\Delta L(p)$ is the **Highest-Unlock Parcel**.

---

## 6.2 Contradiction Engine
The Contradiction Engine evaluates incoming source facts deterministically without heuristic guessing:

1. **Title Conflict**:
   $$\text{Owner}(\text{RoR}) \ne \text{Claimant}(\text{FieldSurvey}) \implies \text{CRITICAL Exception (EX-TITLE)}$$
   *Action*: Blocks Section 23/30 Award Finalization; routes to Competent Authority for hearing speaking order.

2. **Area Discrepancy**:
   $$\frac{|\text{Area}(\text{CadastralMap}) - \text{Area}(\text{RoR})|}{\text{Area}(\text{RoR})} \times 100 > \tau \implies \text{Exception (EX-AREA)}$$
   *Thresholds*: $\tau = 1.0\%$ generates a WARNING; $\tau > 5.0\%$ generates a CRITICAL blocking exception.

3. **Active Judicial Injunction**:
   $$\text{HasActiveStay}(\text{eCourts/RCCMS}) = \text{True} \implies \text{CRITICAL Exception (EX-STAY)}$$
   *Action*: Blocks Possession and Construction; sets 48-hour SLA for Legal Officer review.

---

## 6.3 No Family Invisible Rule (SIA Safeguard)
Guarantees that vulnerable, non-titleholder, or tenant families are never bypassed between SIA and construction:
$$\text{Universe}_{\text{affected}} = \text{SIA}_{\text{survey}} \cup \text{FieldSurveys} \cup \text{TenancyRegistries} \cup \text{FRARights}$$

For every household $h \in \text{Universe}_{\text{affected}}$:
$$\text{Accounted}(h) = \text{HasAward}(h) \lor \text{HasRNR}(h) \lor (\text{Ineligible}(h) \land \text{SpeakingOrderPresent}(h))$$

If $\text{Accounted}(h) = \text{False}$, the system emits a **CRITICAL Exception (EX-SIA-MISSING)** and halts the Award Gate for that project package.

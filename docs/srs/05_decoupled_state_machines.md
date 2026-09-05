# Chapter 5: Decoupled State Machines

## 5.1 The Anti-Dashboard Principle
A core failure of traditional acquisition software is the single "project status" dropdown. In reality, a parcel may be physically possessed while its compensation payment is failed, or an affected family remains unhoused. DHARTI models reality as **four independent, asynchronous state machines**:

```
PARCEL
[Candidate] ──> [Notified] ──> [Awarded] ──> [PossessionVerified] ──> [ConstructionReady]

CLAIMANT
[ObservedInSIA] ──> [IdentityResolved] ──> [InterestVerified] ──> [EntitlementMapped] ──> [Resolved]

PAYMENT
[ObligationCreated] ──> [Sanctioned] ──> [PFMSInstructed] ──> [BankAcknowledged] ──> [ReceiptConfirmed]

R&R
[PlanApproved] ──> [ServiceDelivered] ──> [FamilyVerified] ──> [RelocationSafe] ──> [OutcomeSustained]
```

## 5.2 State Transition Requirements

### 1. Parcel Lifecycle
| State | Required Evidence to Advance |
| :--- | :--- |
| **Candidate** | Alignment corridor centerline intersection with cadastral polygon. |
| **Notified** | Gazette notification reference + frozen cadastral geometry snapshot. |
| **Awarded** | Signed Section 23/30 award declaration + compensation calculation. |
| **PossessionVerified** | Geotagged/timestamped boundary photos + witness panchnama. |
| **ConstructionReady** | Paid compensation confirmed + R&R relocation safe + no court stay. |

### 2. Claimant Lifecycle
| State | Required Evidence to Advance |
| :--- | :--- |
| **ObservedInSIA** | Enumeration event by field survey team. |
| **IdentityResolved** | Tokenized demographic match against beneficiary registry. |
| **InterestVerified** | Title deed, tenancy agreement, or recognized livelihood dependency. |
| **EntitlementMapped** | Monetary compensation or rehabilitation unit assignment. |
| **Resolved** | Receipt confirmed or speaking order of ineligibility recorded. |

### 3. Payment Lifecycle
| State | Required Evidence to Advance |
| :--- | :--- |
| **ObligationCreated** | Finalized statutory award amount. |
| **Sanctioned** | Competent Authority financial sanction order. |
| **PFMSInstructed** | Signed electronic payment file dispatched to treasury gateway. |
| **BankAcknowledged** | Core Banking Solution (CBS) transaction settlement reference. |
| **ReceiptConfirmed** | Beneficiary bank credit acknowledgement or Section 77 deposit. |

### 4. R&R Lifecycle
| State | Required Evidence to Advance |
| :--- | :--- |
| **PlanApproved** | Statutory approval of Resettlement Scheme. |
| **ServiceDelivered** | House allotment, land for land, or resettlement grant handed over. |
| **FamilyVerified** | Ground verification of family relocation. |
| **RelocationSafe** | Basic amenities verified at resettlement center. |
| **OutcomeSustained** | 3-month and 6-month livelihood continuity audits completed. |

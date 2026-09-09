#ifndef DHARTI_SERVICES_EXPLANATORY_QUERY_ENGINE_HPP
#define DHARTI_SERVICES_EXPLANATORY_QUERY_ENGINE_HPP

#include <string>
#include <vector>
#include <memory>
#include <map>

#include "dharti/models/evidence.hpp"
#include "dharti/models/parcel.hpp"
#include "dharti/models/claimant.hpp"
#include "dharti/services/pci_engine.hpp"
#include "dharti/services/contradiction_engine.hpp"
#include "dharti/services/sia_inclusion_engine.hpp"
#include "dharti/services/payment_reconciler.hpp"

namespace dharti {
namespace services {

/**
 * @brief Structured answer synthesized by the Explanatory Query Engine.
 */
struct ExplanatoryResponse {
    std::string query_text;
    std::string category;              // "BOTTLENECK", "LEGAL_STAY", "SIA_COMPLIANCE", "PAYMENT_RECONCILIATION", "CLEARANCE", "PCI_OPTIMIZATION"
    std::string direct_answer;
    std::string statutory_authority;   // e.g. "RFCTLARR Act 2013 §23/§38, NH Act 1956 §3D"
    double current_pci{0.0};
    double potential_pci{0.0};
    double unlock_gain_km{0.0};
    int affected_parcels_count{0};
    std::vector<int64_t> affected_parcel_ids;
    std::vector<std::string> evidence_document_refs;
    std::vector<std::string> actionable_remediation_steps;
    models::GeoLocation location;
};

/**
 * @brief Explanatory Query Engine for authoritative root-cause diagnosis,
 * statutory citations, and remediation guidance across land acquisition corridors.
 */
class ExplanatoryQueryEngine {
public:
    explicit ExplanatoryQueryEngine(
        std::shared_ptr<PCIEngine> pci_engine = nullptr,
        std::shared_ptr<ContradictionEngine> contradiction_engine = nullptr,
        std::shared_ptr<SIAInclusionEngine> sia_engine = nullptr,
        std::shared_ptr<PaymentReconciler> payment_reconciler = nullptr
    );

    /**
     * @brief Interpret natural language query and synthesize structured explanatory diagnosis.
     */
    ExplanatoryResponse answer_query(
        const std::string& natural_query,
        double corridor_length_km,
        const std::vector<Interval>& corridor_intervals,
        const std::vector<models::Household>& sia_households = {}
    );

    /**
     * @brief Format response for CLI console display with box borders and highlights.
     */
    std::string format_console_report(const ExplanatoryResponse& resp) const;

    /**
     * @brief Serialize response into structured JSON string.
     */
    std::string to_json(const ExplanatoryResponse& resp) const;

private:
    std::shared_ptr<PCIEngine> m_pci_engine;
    std::shared_ptr<ContradictionEngine> m_contradiction_engine;
    std::shared_ptr<SIAInclusionEngine> m_sia_engine;
    std::shared_ptr<PaymentReconciler> m_payment_reconciler;
};

} // namespace services
} // namespace dharti

#endif // DHARTI_SERVICES_EXPLANATORY_QUERY_ENGINE_HPP

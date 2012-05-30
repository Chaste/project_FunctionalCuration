#ifdef CHASTE_CVODE
#ifndef ASLANIDI_PURKINJE_MODEL_2009FROMCELLMLCVODENONLHOPITALED_HPP_
#define ASLANIDI_PURKINJE_MODEL_2009FROMCELLMLCVODENONLHOPITALED_HPP_

//! @file
//!
//! This source file was generated from CellML.
//!
//! Model: aslanidi_2009
//!
//! Processed by pycml - CellML Tools in Python
//!     (translators: 11498, pycml: 11498, optimize: 11490)
//! on Wed Feb 23 17:09:47 2011
//!
//! <autogenerated>

#include "AbstractCvodeCell.hpp"
#include "AbstractDynamicallyLoadableEntity.hpp"
#include "AbstractSystemWithOutputs.hpp"
#include "AbstractStimulusFunction.hpp"

class Aslanidi_Purkinje_model_2009FromCellMLCvodeNonLHopitaled : public AbstractCvodeCell, public AbstractDynamicallyLoadableEntity, public AbstractSystemWithOutputs<N_Vector >
{
    //
    // Settable parameters and readable variables
    //
    double var_Ca_i__Ca_i;
    double var_Ca_r__Ca_r;
    double var_environment__time;
    double var_i_Ca_L__g_Ca_L;
    double var_i_Ca_L__i_Ca_L;
    double var_i_Ca_L_d_gate__d;
    double var_i_Ca_L_f2_gate__f2;
    double var_i_Ca_L_f2_gate__tau_f2;
    double var_i_Ca_L_f_Ca2_gate__f_Ca2;
    double var_i_Ca_L_f_Ca2_gate__tau_f_Ca2;
    double var_i_Ca_L_f_Ca_gate__f_Ca;
    double var_i_Ca_L_f_Ca_gate__tau_f_Ca;
    double var_i_Ca_L_f_gate__f;
    double var_i_Ca_L_f_gate__tau_f;
    double var_i_K1__g_K1;
    double var_i_K1__i_K1;
    double var_i_Kr__g_Kr_max;
    double var_i_Kr__i_Kr;
    double var_i_Ks__g_Ks;
    double var_i_Ks__i_Ks;
    double var_i_NaCa__i_NaCa;
    double var_i_NaCa__i_NaCa_max;
    double var_i_Na__g_Na;
    double var_i_Na__i_Na;
    double var_i_Na__perc_reduced_inact_for_IpNa;
    double var_i_Na__shift_INa_inact;
    double var_i_Na_h_gate__h;
    double var_i_Na_h_gate__tau_h;
    double var_i_Na_j_gate__j;
    double var_i_Na_j_gate__tau_j;
    double var_i_Na_m_gate__m;
    double var_i_to_1__g_to_1;
    double var_i_to_1__i_to_1;
    double var_intracellular_ion_concentrations__K_i;
    double var_intracellular_ion_concentrations__Na_i;
    double var_intracellular_ion_concentrations__conc_clamp;
    double var_membrane__V;
    double var_protocol__i_stim;
    double var_q_rel__q_rel;

public:
    double Get_Ca_i__Ca_i();
    double Get_Ca_r__Ca_r();
    double Get_environment__time();
    double Get_i_Ca_L__g_Ca_L();
    double Get_i_Ca_L__i_Ca_L();
    double Get_i_Ca_L_d_gate__d();
    double Get_i_Ca_L_f2_gate__f2();
    double Get_i_Ca_L_f2_gate__tau_f2();
    double Get_i_Ca_L_f_Ca2_gate__f_Ca2();
    double Get_i_Ca_L_f_Ca2_gate__tau_f_Ca2();
    double Get_i_Ca_L_f_Ca_gate__f_Ca();
    double Get_i_Ca_L_f_Ca_gate__tau_f_Ca();
    double Get_i_Ca_L_f_gate__f();
    double Get_i_Ca_L_f_gate__tau_f();
    double Get_i_K1__g_K1();
    double Get_i_K1__i_K1();
    double Get_i_Kr__g_Kr_max();
    double Get_i_Kr__i_Kr();
    double Get_i_Ks__g_Ks();
    double Get_i_Ks__i_Ks();
    double Get_i_NaCa__i_NaCa();
    double Get_i_NaCa__i_NaCa_max();
    double Get_i_Na__g_Na();
    double Get_i_Na__i_Na();
    double Get_i_Na__perc_reduced_inact_for_IpNa();
    double Get_i_Na__shift_INa_inact();
    double Get_i_Na_h_gate__h();
    double Get_i_Na_h_gate__tau_h();
    double Get_i_Na_j_gate__j();
    double Get_i_Na_j_gate__tau_j();
    double Get_i_Na_m_gate__m();
    double Get_i_to_1__g_to_1();
    double Get_i_to_1__i_to_1();
    double Get_intracellular_ion_concentrations__K_i();
    double Get_intracellular_ion_concentrations__Na_i();
    double Get_intracellular_ion_concentrations__conc_clamp();
    double Get_membrane__V();
    double Get_protocol__i_stim();
    double Get_q_rel__q_rel();
    double Get_membrane_fast_sodium_current_shift_inactivation_constant();
    double Get_membrane_fast_sodium_current_conductance_constant();
    double Get_membrane_fast_sodium_current_reduced_inactivation_constant();
    double Get_membrane_rapid_delayed_rectifier_potassium_current_conductance_constant();
    double Get_membrane_L_type_calcium_current_f2_gate_tau_constant();
    double Get_membrane_L_type_calcium_current_conductance_constant();
    double Get_membrane_voltage_constant();
    double Get_membrane_inward_rectifier_potassium_current_conductance_constant();
    double Get_concentration_clamp_onoff_constant();
    double Get_membrane_fast_sodium_current_h_gate_tau_constant();
    double Get_membrane_sodium_calcium_exchanger_current_conductance_constant();
    double Get_membrane_L_type_calcium_current_f_gate_tau_constant();
    double Get_membrane_fast_sodium_current_j_gate_tau_constant();
    double Get_membrane_transient_outward_current_conductance_constant();
    double GetIntracellularCalciumConcentration();
    Aslanidi_Purkinje_model_2009FromCellMLCvodeNonLHopitaled(boost::shared_ptr<AbstractIvpOdeSolver> pOdeSolver /* unused; should be empty */, boost::shared_ptr<AbstractStimulusFunction> pIntracellularStimulus);
    ~Aslanidi_Purkinje_model_2009FromCellMLCvodeNonLHopitaled();
    void VerifyStateVariables();
    double GetIIonic(const std::vector<double>* pStateVariables=NULL);
    void EvaluateYDerivatives(double var_environment__time, const N_Vector rY, N_Vector rDY);
    N_Vector ComputeDerivedQuantities(double var_environment__time, const N_Vector & rY);
    void SolveModel(double endPoint);
};


#endif // ASLANIDI_PURKINJE_MODEL_2009FROMCELLMLCVODENONLHOPITALED_HPP_
#endif // CHASTE_CVODE

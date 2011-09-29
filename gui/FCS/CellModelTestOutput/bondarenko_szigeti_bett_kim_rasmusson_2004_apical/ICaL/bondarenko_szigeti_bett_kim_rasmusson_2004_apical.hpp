#ifdef CHASTE_CVODE
#ifndef DYNAMICBONDARENKO_SZIGETI_BETT_KIM_RASMUSSON_2004_APICALFROMCELLMLCVODE_HPP_
#define DYNAMICBONDARENKO_SZIGETI_BETT_KIM_RASMUSSON_2004_APICALFROMCELLMLCVODE_HPP_

//! @file
//! 
//! This source file was generated from CellML.
//! 
//! Model: bondarenko_2004_apical
//! 
//! Processed by pycml - CellML Tools in Python
//!     (translators: 12434, pycml: 12383, optimize: 12408)
//! on Fri May 27 16:02:41 2011
//! 
//! <autogenerated>

#include "AbstractCvodeCell.hpp"
#include "AbstractDynamicallyLoadableEntity.hpp"
#include "AbstractSystemWithOutputs.hpp"
#include "AbstractStimulusFunction.hpp"

class Dynamicbondarenko_szigeti_bett_kim_rasmusson_2004_apicalFromCellMLCvode : public AbstractCvodeCell, public AbstractDynamicallyLoadableEntity, public AbstractSystemWithOutputs<N_Vector >
{
    // 
    // Settable parameters and readable variables
    // 
    double var_chaste_interface__calcium_concentration__CaJSR;
    double var_calcium_fluxes__J_rel;
    double var_chaste_interface__calcium_concentration__Cai;
    double var_chaste_interface__potassium_concentration__Ki;
    double var_chaste_interface__sodium_concentration__Nai;
    double var_chaste_interface__calcium_concentration__Cass;
    double var_chaste_interface__i_ionic;
    double var_chaste_interface__protocol__i_CaL;
    double var_membrane__Cm;
    double var_fast_sodium_current__i_Na;
    double var_time_independent_potassium_current__i_K1;
    double var_rapid_delayed_rectifier_potassium_current__i_Kr;
    double var_slow_delayed_rectifier_potassium_current__i_Ks;
    double var_sodium_calcium_exchange_current__i_NaCa;
    double var_membrane__i_stim;
    double var_fast_transient_outward_potassium_current__i_Kto_f;
    double var_chaste_interface__environment__time;
    
public:
    double Get_chaste_interface__calcium_concentration__CaJSR();
    double Get_calcium_fluxes__J_rel();
    double Get_chaste_interface__calcium_concentration__Cai();
    double Get_chaste_interface__potassium_concentration__Ki();
    double Get_chaste_interface__sodium_concentration__Nai();
    double Get_chaste_interface__calcium_concentration__Cass();
    double Get_chaste_interface__i_ionic();
    double Get_chaste_interface__protocol__i_CaL();
    double Get_membrane__Cm();
    double Get_fast_sodium_current__i_Na();
    double Get_time_independent_potassium_current__i_K1();
    double Get_rapid_delayed_rectifier_potassium_current__i_Kr();
    double Get_slow_delayed_rectifier_potassium_current__i_Ks();
    double Get_sodium_calcium_exchange_current__i_NaCa();
    double Get_membrane__i_stim();
    double Get_fast_transient_outward_potassium_current__i_Kto_f();
    double Get_chaste_interface__environment__time();
    double Get_SR_leak_current_max_constant();
    double Get_SR_release_current_max_constant();
    double Get_SR_uptake_current_max_constant();
    double Get_concentration_clamp_onoff_constant();
    double Get_extracellular_calcium_concentration_constant();
    double Get_extracellular_potassium_concentration_constant();
    double Get_membrane_L_type_calcium_current_conductance_constant();
    double Get_membrane_fast_sodium_current_conductance_constant();
    double Get_membrane_fast_sodium_current_shift_inactivation_constant();
    double Get_membrane_inward_rectifier_potassium_current_conductance_constant();
    double Get_membrane_rapid_delayed_rectifier_potassium_current_conductance_constant();
    double Get_membrane_slow_delayed_rectifier_potassium_current_conductance_constant();
    double Get_membrane_sodium_calcium_exchanger_current_conductance_constant();
    double Get_membrane_transient_outward_current_conductance_constant();
    double Get_membrane_voltage_constant();
    double GetIntracellularCalciumConcentration();
    Dynamicbondarenko_szigeti_bett_kim_rasmusson_2004_apicalFromCellMLCvode(boost::shared_ptr<AbstractIvpOdeSolver> pOdeSolver /* unused; should be empty */, boost::shared_ptr<AbstractStimulusFunction> pIntracellularStimulus);
    ~Dynamicbondarenko_szigeti_bett_kim_rasmusson_2004_apicalFromCellMLCvode();
    void VerifyStateVariables();
    double GetIIonic(const std::vector<double>* pStateVariables=NULL);
    void EvaluateRhs(double var_chaste_interface__environment__time, const N_Vector rY, N_Vector rDY);
    N_Vector ComputeDerivedQuantities(double var_chaste_interface__environment__time, const N_Vector & rY);
};


#endif // DYNAMICBONDARENKO_SZIGETI_BETT_KIM_RASMUSSON_2004_APICALFROMCELLMLCVODE_HPP_
#endif // CHASTE_CVODE

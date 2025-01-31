/** @file
*****************************************************************************

Declaration of interfaces for LegoGroth


*****************************************************************************
* @author     Matteo Campanelli
* @copyright  MIT license (see LICENSE file)
*****************************************************************************/

#ifndef R1CS_GG_PPZKSNARK_LEGO_HPP_
#define R1CS_GG_PPZKSNARK_LEGO_HPP_
 
#include <memory>

#include <libff/algebra/curves/public_params.hpp>

#include <libsnark/relations/constraint_satisfaction_problems/r1cs/r1cs.hpp>
#include <libsnark/relations/constraint_satisfaction_problems/r1cs/examples/r1cs_examples.hpp>


#include <libsnark/common/data_structures/accumulation_vector.hpp>
#include <libsnark/knowledge_commitment/knowledge_commitment.hpp>

#include <libsnark/zk_proof_systems/ppzksnark/r1cs_gg_ppzksnark/r1cs_gg_ppzksnark.hpp>


#include <libsnark/zk_proof_systems/ppzksnark/r1cs_gg_ppzksnark/cplink.hpp>

namespace libsnark {


/* commitment-related */
template<typename ppT>
using lego_ck = std::vector<libff::G1<ppT>>;


template<typename ppT>
    lego_ck<ppT> lego_gen_ck(auto opn_size);

template<typename ppT>
    auto lego_commit(const lego_ck<ppT> &ck, const auto &opn) {
        return multiExpMA< libff::G1<ppT>, libff::Fr<ppT> >(ck, opn);
}

/* input related objects */

template<typename FieldT>
using lego_pub_input = std::vector<FieldT>;

template<typename FieldT>
using lego_opn_input = std::vector<FieldT>;

template<typename FieldT>
using lego_uncomm_aux_input = std::vector<FieldT>;


void _lego_set_slice(auto &dst, const auto &src, size_t start, size_t end)
{
    dst.assign( std::begin(src)+start, std::begin(src)+end);
}


template<typename FieldT>
class lego_constraint_system {
    public:
        r1cs_constraint_system<FieldT> cs;
        size_t comm_input_size; // |cs.primary_input_size| = |public_x| + comm_input_size 

        size_t x_size() const {
            return cs.primary_input_size-comm_input_size;
        }

        size_t opn_size() const {
            return comm_input_size;
        }
};


template<typename ppT>
struct lego_example {
    using FieldT = typename libff::Fr<ppT>;
    using CommT = typename libff::G1<ppT>;

    lego_constraint_system<FieldT> lego_cs;

    lego_pub_input<FieldT> x;
    lego_opn_input<FieldT> opn;
    lego_uncomm_aux_input<FieldT> omega;

    CommT cm;

    lego_ck<ppT> ck;

    lego_constraint_system<FieldT> r1cs() const {
        return lego_cs;
    }

};

// FIXME
template<typename ppT>
auto gen_lego_example(auto cs, auto pub_input, auto committable_input, auto omega) {
    lego_example<ppT> lego_ex;
    lego_ex.lego_cs.cs = cs;
    lego_ex.lego_cs.comm_input_size = committable_input.size();

    lego_ex.ck = lego_gen_ck<ppT>(committable_input.size());
    lego_ex.cm = lego_commit<ppT>(lego_ex.ck, committable_input);
    lego_ex.opn = committable_input;
    lego_ex.omega = omega;

    return lego_ex;
}
        
template<typename ppT>
lego_example<ppT> generate_lego_example_with_field_input(const size_t num_constraints,
                                                            const size_t size_pub_input,
                                                            const size_t size_comm_input)
{
    lego_example<ppT> lego_ex;
    size_t sz_pub_plus_comm = size_pub_input+size_comm_input;
    r1cs_example< libff::Fr<ppT> > r1cs_ex =
     generate_r1cs_example_with_field_input<libff::Fr<ppT> >(num_constraints, sz_pub_plus_comm);

    _lego_set_slice(lego_ex.x, r1cs_ex.primary_input, 0 , size_pub_input);
    _lego_set_slice(lego_ex.opn, r1cs_ex.primary_input, size_pub_input, sz_pub_plus_comm);

    
    lego_ex.ck = lego_gen_ck<ppT>(size_comm_input);
    lego_ex.cm = lego_commit<ppT>(lego_ex.ck, lego_ex.opn);

    lego_ex.omega = r1cs_ex.auxiliary_input;

    lego_ex.lego_cs.cs = r1cs_ex.constraint_system;
    lego_ex.lego_cs.comm_input_size = size_comm_input;

    return lego_ex;
}


/* proof related objects */



template<typename ppT>
class lego_keypair {
    public: 

    std::vector<libff::G1<ppT>> ck;
   
    r1cs_gg_ppzksnark_keypair<ppT> gro16keypair; 
    r1cs_gg_ppzksnark_processed_verification_key<ppT> pvk;

    libff::G1<ppT> eta_delta_inv_g1;
    libff::G1<ppT> eta_gamma_inv_g1;

    std::vector<libff::G1<ppT>> gamma_ABC_g1_x, gamma_ABC_g1_u;


    cplink_key<ppT> lnk_key;

    lego_keypair( r1cs_gg_ppzksnark_keypair<ppT> _gro16keypair) :
        gro16keypair(_gro16keypair)
    {
        libff::print_header("Preprocess verification key");
        pvk = r1cs_gg_ppzksnark_verifier_process_vk<ppT>(gro16keypair.vk);
    }

    auto gro16pk() const {
        return gro16keypair.pk;
    }

    auto gro16pvk() const {
        // return preprocessed vk
        return pvk;
    }
};

template<typename ppT>
struct lego_proof {
    r1cs_gg_ppzksnark_proof<ppT> gro16prf;
    libff::G1<ppT> g_D;

    // cplink_prf
    libff::G1<ppT> lnk_prf;
};

template<typename ppT>
    lego_keypair<ppT> lego_kg(const auto &ck, const auto &cs);

template<typename ppT>
    lego_proof<ppT> lego_prv(const auto &kp, const auto &x, const auto &cm, const auto &opn,  const auto &omega);

template<typename ppT>
    bool lego_vfy(const auto &kp, const auto &x, const auto &cm, const auto &prf);

}

#include <libsnark/zk_proof_systems/ppzksnark/r1cs_gg_ppzksnark/lego.tcc>

#endif // R1CS_GG_PPZKSNARK_LEGO_HPP_

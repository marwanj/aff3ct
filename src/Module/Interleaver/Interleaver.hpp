/*!
 * \file
 * \brief Interleaves or deinterleaves a vector.
 *
 * \section LICENSE
 * This file is under MIT license (https://opensource.org/licenses/MIT).
 */
#ifndef INTERLEAVER_HPP_
#define INTERLEAVER_HPP_

#include <stdexcept>
#include <typeinfo>
#include <string>
#include <vector>
#include "Tools/Perf/MIPP/mipp.h"

#include "Module/Module.hpp"

namespace aff3ct
{
namespace module
{
/*!
 * \class Interleaver_i
 *
 * \brief Interleaves or deinterleaves a vector.
 *
 * \tparam T: type of the integers used in the lookup tables to store indirections.
 *
 * Please use Interleaver for inheritance (instead of Interleaver_i)
 */
template <typename T = int>
class Interleaver_i : public Module
{
protected:
	mipp::vector<T> pi;     /*!< Lookup table for the interleaving process */
	mipp::vector<T> pi_inv; /*!< Lookup table for the deinterleaving process */
	
public:
	/*!
	 * \brief Constructor.
	 *
	 * \param size:     number of the data to interleave or to deinterleave.
	 * \param n_frames: number of frames to process in the Interleaver.
	 * \param name:     Interleaver's name.
	 */
	Interleaver_i(const int size, const int n_frames = 1, const std::string name = "Interleaver_i")
	: Module(n_frames, name), pi(size), pi_inv(size)
	{
		if (size <= 0)
			throw std::invalid_argument("aff3ct::module::Interleaver: \"size\" has to be greater than 0.");
	}

	/*!
	 * \brief Destructor.
	 */
	virtual ~Interleaver_i()
	{
	}

	/*!
	 * \brief Gets the lookup table required for the interleaving process.
	 *
	 * \return a vector of indirections.
	 */
	mipp::vector<T> get_lookup_table() const
	{
		return pi;
	}

	/*!
	 * \brief Gets the lookup table required for the deinterleaving process.
	 *
	 * \return a vector of indirections.
	 */
	mipp::vector<T> get_lookup_table_inverse() const
	{
		return pi_inv;
	}

	unsigned size() const
	{
		return pi.size();
	}

	/*!
	 * \brief Interleaves a vector.
	 *
	 * \tparam D: type of data in the input and the output vectors.
	 *
	 * \param natural_vec:      an input vector in the natural domain.
	 * \param interleaved_vec:  an output vector in the interleaved domain.
	 * \param frame_reordering: true means that the frames have been reordered for efficient SIMD computations. In this
	 *                          case the interleaving process is different (true supposes that there is more than one
	 *                          frame to interleave).
	 * \param n_frames:         you should not use this parameter unless you know what you are doing, this parameter
	 *                          redefine the number of frames to interleave specifically in this method.
	 */
	template <typename D>
	inline void interleave(const mipp::vector<D> &natural_vec, 
	                             mipp::vector<D> &interleaved_vec, 
	                       const bool             frame_reordering = false,
	                       const int              n_frames = -1) const
	{
		if (n_frames != -1 && n_frames <= 0)
			throw std::invalid_argument("aff3ct::module::Interleaver: \"n_frames\" has to be greater than 0 "
			                            "(or equal to -1).");

		const int real_n_frames = (n_frames != -1) ? n_frames : this->n_frames;

		if (natural_vec.size() != interleaved_vec.size())
			throw std::length_error("aff3ct::module::Interleaver: \"natural_vec.size()\" has to be equal to "
			                        "\"interleaved_vec.size()\".");

		if (natural_vec.size() < this->size() * real_n_frames)
			throw std::length_error("aff3ct::module::Interleaver: \"natural_vec.size()\" has to be equal or greater "
			                        "than \"this->size()\" * \"real_n_frames\".");

		this->interleave(natural_vec.data(), interleaved_vec.data(), frame_reordering, real_n_frames);
	}

	template <typename D>
	inline void interleave(const D *natural_vec, D *interleaved_vec,
	                       const bool frame_reordering = false,
	                       const int  n_frames = -1) const
	{
		const int real_n_frames = (n_frames != -1) ? n_frames : this->n_frames;
		this->_interleave(natural_vec, interleaved_vec, pi, frame_reordering, real_n_frames);
	}

	/*!
	 * \brief Deinterleaves a vector.
	 *
	 * \tparam D: type of data in the input and the output vectors.
	 *
	 * \param interleaved_vec:  an input vector in the interleaved domain.
	 * \param natural_vec:      an output vector in the natural domain.
	 * \param frame_reordering: true means that the frames have been reordered for efficient SIMD computations. In this
	 *                          case the deinterleaving process is different (true supposes that there is more than one
	 *                          frame to deinterleave).
	 * \param n_frames:         you should not use this parameter unless you know what you are doing, this parameter
	 *                          redefine the number of frames to deinterleave specifically in this method.
	 */
	template <typename D>
	inline void deinterleave(const mipp::vector<D> &interleaved_vec, 
	                               mipp::vector<D> &natural_vec, 
	                         const bool             frame_reordering = false,
	                         const int              n_frames = -1) const
	{
		if (n_frames != -1 && n_frames <= 0)
			throw std::invalid_argument("aff3ct::module::Interleaver: \"n_frames\" has to be greater than 0 "
			                            "(or equal to -1).");

		const int real_n_frames = (n_frames != -1) ? n_frames : this->n_frames;

		if (natural_vec.size() != interleaved_vec.size())
			throw std::length_error("aff3ct::module::Interleaver: \"natural_vec.size()\" has to be equal to "
			                        "\"interleaved_vec.size()\".");

		if (natural_vec.size() < this->size() * real_n_frames)
			throw std::length_error("aff3ct::module::Interleaver: \"natural_vec.size()\" has to be equal or greater "
			                        "than \"this->size()\" * \"real_n_frames\".");

		this->_interleave(interleaved_vec.data(), natural_vec.data(), pi_inv, frame_reordering, real_n_frames);
	}

	template <typename D>
	inline void _deinterleave(const D *interleaved_vec, D *natural_vec,
	                          const bool frame_reordering = false,
	                          const int  n_frames = -1) const
	{
		const int real_n_frames = (n_frames != -1) ? n_frames : this->n_frames;
		this->_interleave(interleaved_vec, natural_vec, pi_inv, frame_reordering, real_n_frames);
	}

	/*!
	 * \brief Compares two interleavers: the comparison only considers the lookup tables.
	 *
	 * \param interleaver: an other interleaver.
	 *
	 * \return true if the two interleavers are the same (if they have the same lookup tables), false otherwise.
	 */
	bool operator==(Interleaver_i<T> &interleaver) const
	{
		if (interleaver.pi.size() != this->pi.size())
			return false;

		if (interleaver.pi_inv.size() != this->pi_inv.size())
			return false;

		for (auto i = 0; i < (int)this->pi.size(); i++)
			if (this->pi[i] != interleaver.pi[i])
				return false;

		for (auto i = 0; i < (int)this->pi_inv.size(); i++)
			if (this->pi_inv[i] != interleaver.pi_inv[i])
				return false;

		return true;
	}

	bool operator!=(Interleaver_i<T> &interleaver) const
	{
		return !(*this == interleaver);
	}

	/*!
	 * \brief Generates the interleaving and deinterleaving lookup tables. This method defines the interleaver and have
	 *        to be called in the constructor.
	 */
	virtual void gen_lookup_tables() = 0; // to implement

private:
	template <typename D>
	inline void _interleave(const D *in_vec, D *out_vec,
	                        const mipp::vector<T> &lookup_table,
	                        const bool frame_reordering,
	                        const int  n_frames) const
	{
		const auto frame_size = (int)lookup_table.size();

		if (frame_reordering)
		{
			// vectorized interleaving
			if (n_frames == mipp::nElReg<D>())
				for (auto i = 0; i < frame_size; i++)
					mipp::store<D>(&out_vec[i * mipp::nElReg<D>()], 
					               mipp::load<D>(&in_vec[lookup_table[i] * mipp::nElReg<D>()]));
			else
				for (auto i = 0; i < frame_size; i++)
				{
					const auto off1 =              i  * n_frames;
					const auto off2 = lookup_table[i] * n_frames;
					for (auto f = 0; f < n_frames; f++)
						out_vec[off1 +f] = in_vec[off2 +f];
				}
		}
		else // sequential interleaving
		{
			// TODO: vectorize this code with the new AVX gather instruction
			for (auto f = 0; f < n_frames; f++)
			{
				const auto off = f * frame_size;
				for (auto i = 0; i < frame_size; i++)
					out_vec[off + i] = in_vec[off + lookup_table[i]];
			}
		}
	}
};
}
}

#include "SC_Interleaver.hpp"

#endif /* INTERLEAVER_HPP_ */

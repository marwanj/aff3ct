#include <sstream>
#include <string>

#include "Tools/Exception/exception.hpp"
#include "Tools/Noise/Sigma.hpp"
#include "Module/Channel/Channel.hpp"

namespace aff3ct
{
namespace module
{

template <typename R>
Task& Channel<R>
::operator[](const chn::tsk t)
{
	return Module::operator[]((size_t)t);
}

template <typename R>
Socket& Channel<R>
::operator[](const chn::sck::add_noise s)
{
	return Module::operator[]((size_t)chn::tsk::add_noise)[(size_t)s];
}

template <typename R>
Socket& Channel<R>
::operator[](const chn::sck::add_noise_wg s)
{
	return Module::operator[]((size_t)chn::tsk::add_noise_wg)[(size_t)s];
}

template <typename R>
Channel<R>
::Channel(const int N, const tools::Noise<R>* n, const int n_frames)
: Module(n_frames), N(N), n(n), noise(this->N * this->n_frames, 0)
{
	const std::string name = "Channel";
	this->set_name(name);
	this->set_short_name(name);

	if (N <= 0)
	{
		std::stringstream message;
		message << "'N' has to be greater than 0 ('N' = " << N << ").";
		throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
	}

	auto &p1 = this->create_task("add_noise");
	auto p1s_X_N = this->template create_socket_in <R>(p1, "X_N", this->N);
	auto p1s_Y_N = this->template create_socket_out<R>(p1, "Y_N", this->N);
	this->create_codelet(p1, [this, p1s_X_N, p1s_Y_N](Task &t) -> int
	{
		this->add_noise(static_cast<R*>(t[p1s_X_N].get_dataptr()),
		                static_cast<R*>(t[p1s_Y_N].get_dataptr()));

		return 0;
	});

	auto &p2 = this->create_task("add_noise_wg");
	auto p2s_X_N = this->template create_socket_in <R>(p2, "X_N", this->N);
	auto p2s_H_N = this->template create_socket_out<R>(p2, "H_N", this->N);
	auto p2s_Y_N = this->template create_socket_out<R>(p2, "Y_N", this->N);
	this->create_codelet(p2, [this, p2s_X_N, p2s_H_N, p2s_Y_N](Task &t) -> int
	{
		this->add_noise_wg(static_cast<R*>(t[p2s_X_N].get_dataptr()),
		                   static_cast<R*>(t[p2s_H_N].get_dataptr()),
		                   static_cast<R*>(t[p2s_Y_N].get_dataptr()));

		return 0;
	});
}

template <typename R>
Channel<R>
::Channel(const int N, const int n_frames)
: Channel(N, nullptr, n_frames)
{
}

template <typename R>
int Channel<R>
::get_N() const
{
	return this->N;
}

template <typename R>
const std::vector<R>& Channel<R>
::get_noise() const
{
	return noise;
}

template <typename R>
void Channel<R>
::set_noise(const tools::Noise<R>& n)
{
	this->n = &n;
	this->check_noise();
}

template <typename R>
void Channel<R>
::set_seed(const int seed)
{
	// do nothing in the general case, this method has to be overrided
}

template<typename R>
const tools::Noise<R>& Channel<R>
::current_noise() const
{
	return *this->n;
}

template <typename R>
template <class A>
void Channel<R>
::add_noise(const std::vector<R,A>& X_N, std::vector<R,A>& Y_N, const int frame_id)
{
	if (X_N.size() != Y_N.size())
	{
		std::stringstream message;
		message << "'X_N.size()' has to be equal to 'Y_N.size()' ('X_N.size()' = "
		        << X_N.size() << ", 'Y_N.size()' = " << Y_N.size() << ").";
		throw tools::length_error(__FILE__, __LINE__, __func__, message.str());
	}

	if (this->N * this->n_frames != (int)X_N.size())
	{
		std::stringstream message;
		message << "'X_N.size()' has to be equal to 'N' * 'n_frames' ('X_N.size()' = "
		        << X_N.size() << ", 'N' = " << this->N << ", 'n_frames' = " << this->n_frames << ").";
		throw tools::length_error(__FILE__, __LINE__, __func__, message.str());
	}

	if (this->N * this->n_frames != (int)Y_N.size())
	{
		std::stringstream message;
		message << "'Y_N.size()' has to be equal to 'N' * 'n_frames' ('Y_N.size()' = "
		        << Y_N.size() << ", 'N' = " << this->N << ", 'n_frames' = " << this->n_frames << ").";
		throw tools::length_error(__FILE__, __LINE__, __func__, message.str());
	}

	if (frame_id != -1 && frame_id >= this->n_frames)
	{
		std::stringstream message;
		message << "'frame_id' has to be equal to '-1' or to be smaller than 'n_frames' ('frame_id' = "
		        << frame_id << ", 'n_frames' = " << this->n_frames << ").";
		throw tools::length_error(__FILE__, __LINE__, __func__, message.str());
	}

	this->add_noise(X_N.data(), Y_N.data(), frame_id);
}

template <typename R>
void Channel<R>
::add_noise(const R *X_N, R *Y_N, const int frame_id)
{
	const auto f_start = (frame_id < 0) ? 0 : frame_id % this->n_frames;
	const auto f_stop  = (frame_id < 0) ? this->n_frames : f_start +1;

	for (auto f = f_start; f < f_stop; f++)
		this->_add_noise(X_N + f * this->N,
		                 Y_N + f * this->N,
		                 f);
}

template <typename R>
template <class A>
void Channel<R>
::add_noise_wg(const std::vector<R,A>& X_N, std::vector<R,A>& H_N, std::vector<R,A>& Y_N, const int frame_id)
{
	if (X_N.size() != Y_N.size() || Y_N.size() != H_N.size())
	{
		std::stringstream message;
		message << "'X_N.size()' has to be equal to 'Y_N.size()' and 'H_N.size()' ('X_N.size()' = "
		        << X_N.size() << ", 'Y_N.size()' = " << Y_N.size() << ", 'H_N.size()' = " << H_N.size() << ").";
		throw tools::length_error(__FILE__, __LINE__, __func__, message.str());
	}

	if (this->N * this->n_frames != (int)X_N.size())
	{
		std::stringstream message;
		message << "'X_N.size()' has to be equal to 'N' * 'n_frames' ('X_N.size()' = "
		        << X_N.size() << ", 'N' = " << this->N << ", 'n_frames' = " << this->n_frames << ").";
		throw tools::length_error(__FILE__, __LINE__, __func__, message.str());
	}

	if (this->N * this->n_frames != (int)Y_N.size())
	{
		std::stringstream message;
		message << "'Y_N.size()' has to be equal to 'N' * 'n_frames' ('Y_N.size()' = "
		        << Y_N.size() << ", 'N' = " << this->N << ", 'n_frames' = " << this->n_frames << ").";
		throw tools::length_error(__FILE__, __LINE__, __func__, message.str());
	}

	if (this->N * this->n_frames != (int)H_N.size())
	{
		std::stringstream message;
		message << "'H_N.size()' has to be equal to 'N' * 'n_frames' ('H_N.size()' = "
		        << H_N.size() << ", 'N' = " << this->N << ", 'n_frames' = " << this->n_frames << ").";
		throw tools::length_error(__FILE__, __LINE__, __func__, message.str());
	}

	if (frame_id != -1 && frame_id >= this->n_frames)
	{
		std::stringstream message;
		message << "'frame_id' has to be equal to '-1' or to be smaller than 'n_frames' ('frame_id' = "
		        << frame_id << ", 'n_frames' = " << this->n_frames << ").";
		throw tools::length_error(__FILE__, __LINE__, __func__, message.str());
	}

	this->add_noise_wg(X_N.data(), H_N.data(), Y_N.data(), frame_id);
}

template <typename R>
void Channel<R>
::add_noise_wg(const R *X_N, R *Y_N, R *H_N, const int frame_id)
{
	const auto f_start = (frame_id < 0) ? 0 : frame_id % this->n_frames;
	const auto f_stop  = (frame_id < 0) ? this->n_frames : f_start +1;

	for (auto f = f_start; f < f_stop; f++)
		this->_add_noise_wg(X_N + f * this->N,
		                    H_N + f * this->N,
		                    Y_N + f * this->N,
		                    f);
}

template <typename R>
void Channel<R>
::_add_noise(const R *X_N, R *Y_N, const int frame_id)
{
	throw tools::unimplemented_error(__FILE__, __LINE__, __func__);
}

template <typename R>
void Channel<R>
::_add_noise_wg(const R *X_N, R *H_N, R *Y_N, const int frame_id)
{
	throw tools::unimplemented_error(__FILE__, __LINE__, __func__);
}

template<typename R>
void Channel<R>
::check_noise()
{
	if (this->n == nullptr)
	{
		std::stringstream message;
		message << "No noise has been set.";
		throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
	}
}

}
}

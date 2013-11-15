/**
 * @file bob/sp/DCT1DNumpy.h
 * @date Thu Nov 14 18:16:30 CET 2013
 * @author Laurent El Shafey <Laurent.El-Shafey@idiap.ch>
 *
 * @brief Implement a blitz-based 1D Discrete Cosine Transform using a 1D FFT
 * functions
 *
 * Copyright (C) 2011-2013 Idiap Research Institute, Martigny, Switzerland
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef BOB_SP_DCT1DNUMPY_H
#define BOB_SP_DCT1DNUMPY_H

#include <complex>
#include <blitz/array.h>
#include <boost/shared_ptr.hpp>
#include <bob/sp/FFT1DNumpy.h>


namespace bob { namespace sp {
/**
 * @ingroup SP
 * @{
 */

/**
 * @brief This class implements a 1D Discrete Fourier Transform based on 
 * the kiss DCT library. It is used as a base class for DCT1D and
 * IDCT1D classes.
 */
class DCT1DNumpyAbstract
{
  public:
    /**
     * @brief Destructor
     */
    virtual ~DCT1DNumpyAbstract();

    /**
     * @brief Assignment operator
     */
    DCT1DNumpyAbstract& operator=(const DCT1DNumpyAbstract& other);

    /**
     * @brief Equal operator
     */
    bool operator==(const DCT1DNumpyAbstract& other) const;

    /**
     * @brief Not equal operator
     */
    bool operator!=(const DCT1DNumpyAbstract& other) const;

    /**
     * @brief process an array by applying the DCT
     */
    virtual void operator()(const blitz::Array<double,1>& src, 
      blitz::Array<double,1>& dst) const;

    /**
     * @brief Getters
     */
    size_t getLength() const { return m_length; }
    /**
     * @brief Setters
     */
    virtual void setLength(const size_t length);

  protected:
    /**
     * @brief Constructor
     */
    DCT1DNumpyAbstract(const size_t length);

    /**
     * @brief Copy constructor
     */
    DCT1DNumpyAbstract(const DCT1DNumpyAbstract& other);

    /**
     * @brief process an array assuming that all the 'check' are done
     */
    virtual void processNoCheck(const blitz::Array<double,1>& src,
      blitz::Array<double,1>& dst) const = 0;
    /**
     * @brief Initialize the normalization factors
     */
    virtual void initNormFactors();
    /**
     * @brief initializes the working array of exponentials
     */
    virtual void initWorkingArray() = 0;

    /**
     * Private attributes
     */
    size_t m_length;
    double m_sqrt_1byl;
    double m_sqrt_2byl;
    blitz::Array<std::complex<double>,1> m_working_array;
};


/**
 * @brief This class implements a direct 1D Discrete Fourier Transform 
 * based on the kiss DCT library
 */
class DCT1DNumpy: public DCT1DNumpyAbstract
{
  public:
    /**
     * @brief Constructor
     */ 
    DCT1DNumpy(const size_t length);

    /**
     * @brief Copy constructor
     */
    DCT1DNumpy(const DCT1DNumpy& other);

    /**
     * @brief Destructor
     */
    virtual ~DCT1DNumpy();

    /**
     * @brief Assignment operator
     */
    DCT1DNumpy& operator=(const DCT1DNumpy& other);

    /**
     * @brief Setters
     */
    virtual void setLength(const size_t length);

  private:
    /**
     * @brief initializes the working array of exponentials
     */
    void initWorkingArray();

    /**
     * @brief process an array assuming that all the 'check' are done
     */
    virtual void processNoCheck(const blitz::Array<double,1>& src,
      blitz::Array<double,1>& dst) const;

    /**
     * Private attributes
     */
    bob::sp::FFT1DNumpy m_fft;
    mutable blitz::Array<std::complex<double>,1> m_buffer_1;
    mutable blitz::Array<std::complex<double>,1> m_buffer_2;
};


/**
 * @brief This class implements a inverse 1D Discrete Fourier Transform 
 * based on the kiss DCT library
 */
class IDCT1DNumpy: public DCT1DNumpyAbstract
{
  public:
    /**
     * @brief Constructor
     */ 
    IDCT1DNumpy(const size_t length);

    /**
     * @brief Copy constructor
     */
    IDCT1DNumpy(const IDCT1DNumpy& other);

    /**
     * @brief Destructor
     */
    virtual ~IDCT1DNumpy();

    /**
     * @brief Assignment operator
     */
    IDCT1DNumpy& operator=(const IDCT1DNumpy& other);

    /**
     * @brief Setters
     */
    virtual void setLength(const size_t length);

  private:
    /**
     * @brief initializes the working array of exponentials
     */
    void initWorkingArray();

    /**
     * @brief process an array assuming that all the 'check' are done
     */
    virtual void processNoCheck(const blitz::Array<double,1>& src,
      blitz::Array<double,1>& dst) const;

    /**
     * Private attributes
     */
    bob::sp::IFFT1DNumpy m_ifft;
    mutable blitz::Array<std::complex<double>,1> m_buffer_1;
    mutable blitz::Array<std::complex<double>,1> m_buffer_2;
};

/**
 * @}
 */
}}

#endif /* BOB_SP_DCT1DNUMPY_H */

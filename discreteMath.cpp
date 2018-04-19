/* 
Copyright (c) 2018, Eric Loewenthal
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the organization nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#include <iostream>
#include <cmath>
#include <vector>
#include <stdexcept>
#include <string>
#include "discreteMath.h"



/*
 *          ---=== discreteFilt function definition ===---
 */
/*
 *  Constructor 
 * 
 * Checks that order = aGainIn.length() - 1 =  bGainIn.length() - 1
 * and sets the various parameters.
 * 
 * For safety, the delays are also explicitly reset to zero (these can be
 * overriden later using setState) 
 */
discreteFilt2ndOrder::discreteFilt2ndOrder(const std::vector<double>& aGainIn, const std::vector<double>& bGainIn)
{
    this -> setParams(aGainIn, bGainIn);
    this -> resetState();      
}

/*
 *  Constructor
 * 
 * Sets the gains using a discreteBiquadSectionParams object
 *  
 * For safety, the delays are also explicitly reset to zero (these can be
 * overriden later using setState) 
 */
discreteFilt2ndOrder::discreteFilt2ndOrder(const discreteBiquadSectionParams& config)
{
    this -> setParams(config);
    this -> resetState();
}

/*
 *  Gains setter
 * 
 * Sets the gains to new values, if the vector lengths match the existing
 */
void discreteFilt2ndOrder::setParams(const std::vector<double>& aGainIn, const std::vector<double>& bGainIn)
{
    //Start by checking the order
    /*
     *  We use one more delay than the filter order,
     * because it allows us to reference the delays with the proper superscript
     * (1, 2, 3, ... instead of 0, 1, 2, 3), with delay zero being used for some 
     * intermediate calculations
     */
    if ((3 != aGainIn.size()) || (3 != bGainIn.size()))
    {
        //Oops, caller messed up, throw an exception
        std::length_error mismatchedLengthException("Filter gain vectors must be of length 3!\n");
        throw mismatchedLengthException;
    }

    //On to the useful stuff
    for (int i = 0; i <  3; i++)
    {
        aGains[i] = aGainIn[i];
        bGains[i] = bGainIn[i];
    }
}

/*
 *  Gains setter
 * 
 * Sets the gains according to a discreteBiquadSectionParams object
 */
void discreteFilt2ndOrder::setParams(const discreteBiquadSectionParams& config)
{
    aGains[0] = 1;
    aGains[1] = config.a1;
    aGains[2] = config.a2;
    bGains[0] = config.b0;
    bGains[1] = config.b1;
    bGains[2] = config.b2;
}

/*
 *  resetState resets the delays vector to zero
 */
void discreteFilt2ndOrder::resetState()
{
    for (int i = 0; i <  3; i++)
    {
        delays[i] = 0;
    }
    currOutput = 0; 
}

/*
 *  setState sets the delays to match a new state
 * 
 * The vector is read as follows:
 * 
 * newState[0] - ignored
 * newState[1] - output of first delay
 * newState[2] - output of second delay
 * 
 */
void discreteFilt2ndOrder::setState(const std::vector<double>& newState)
{
    //Start by checking the order
    /*
     *  We use one more delay than the filter order,
     * because it allows us to reference the delays with the proper superscript
     * (1, 2, 3, ... instead of 0, 1, 2, 3), with delay zero being used for some 
     * intermediate calculations
     */
    if (3 != newState.size())
    {
        //Oops, caller messed up, throw an exception
        std::length_error mismatchedLengthException("Filter state vector must be of length 3!\n");
        throw mismatchedLengthException;
    }

    for (int i = 1; i < 3; i++)
    {
        delays[i] = newState[i];
    } 
}

/*
 *  nextSample runs the filter through one sample time, effectively moving
 * forward in time by one quantum
 * 
 * The class parameters *MUST NOT BE ALTERED* while this function is executing.
 * 
 * Thread safety is left up to higher layers, for efficiency reasons.
 */
double discreteFilt2ndOrder::nextSample(double newInput)
{
    //This is the direct form II difference equation
    delays[0] = newInput - aGains[1]*delays[1] - aGains[2]*delays[2];
    currOutput = bGains[0]*delays[0] + bGains[1]*delays[1] + bGains[2]*delays[2];

    //Don't forget to move stuff along the delay sequence
    delays[2] = delays[1];
    delays[1] = delays[0];

    return currOutput;
}


/*
 *       ---=== Generic vector function definitions ===---
 *
 *                  ***** Constructors *****
 * 
 */ 

/*
 *  Initial length constructor
 * 
 *  Constructs the elements vector with initial size vectorLen
 */
genericVector::genericVector(const unsigned int vectorLen) : elements(vectorLen, 0)
{}

/*
 *  Move constructor
 * 
 *  Move an existing genericVector and use it as the elements vector
 */
genericVector::genericVector(genericVector&& toBeMoved)
{
    elements.swap(toBeMoved.elements);
}

/* 
 *  Copy constructor
 *  
 *  Copy an existing genericVector by copying its elements into our own
 */
genericVector::genericVector(const genericVector& toBeCopied)
{
    elements = toBeCopied.elements;
}

/*
 *  Pseudo-copy constructor
 * 
 *  Copy an existing std::vector and use that as the elements vector
 */
genericVector::genericVector(const std::vector<double>& scalars)
{
    elements = scalars; 
}

/*
 *  Pseudo-move constructor
 * 
 *  Move an existing std::vector and use that as the elements vector
 */
genericVector::genericVector(std::vector<double>&& scalars)
{
    elements.swap(scalars); 
}






/*
 *
 *          ***** Operator overloads *****
 * 
 */

/*
 *  Copy assignment
 */
genericVector&  genericVector::operator=(const genericVector& rhs)
{
    if (this != &rhs)
    {
        elements = rhs.elements;
    }
    return *this;
}

/*
 *  Move assignment
 */
genericVector& genericVector::operator=(genericVector&& rhs)
{
    if (this != &rhs)
    {
        elements.swap(rhs.elements);
    }
    return *this;
}

/*
 *  Equality test
 * 
 *  Returns false if the sizes don't match or if a non-matching element is found.
 *  Returns true otherwise
 */
bool genericVector::operator==(const genericVector& rhs)
{
    if (this -> elements.size() == rhs.elements.size())
    {
        for (unsigned int i = 0; i < this -> elements.size(); i++)
        {
            if (this -> elements[i] != rhs.elements[i])
            {
                return false;
            }            
        }
        return true;
    }
    return false;
}

/*
 *  Inequality test
 * 
 * Uses operator== for the heavy lifting
 * 
 * Could potentially be optimized, but who cares...
 */
bool genericVector::operator!=(const genericVector& rhs)
{
    return !(*this==rhs);
}

/*
 *  Compound assignment sum
 * 
 * Sums each element of the vectors and stores in the lhs.
 * 
 * Throws std::length_error if the sizes don't match.
 */
genericVector& genericVector::operator+=(const genericVector& rhs)
{
    if (this->elements.size() != rhs.elements.size())
    {
        //Vector sizes don't match, throw exception
        std::length_error mismatchedLengthException("Vectors to sum must be of same size!\n");
        throw mismatchedLengthException;
    }

    for (unsigned int i = 0; i < this->elements.size(); i++)
    {
        this->elements[i] += rhs.elements[i];
    }
    return *this;

}

/*
 *  Compound assignment subtraction
 * 
 * Subtracts each element of the vectors and stores in the lhs.
 * 
 * Throws std::length_error if the sizes don't match.
 */
genericVector& genericVector::operator-=(const genericVector& rhs)
{
    if (this->elements.size() != rhs.elements.size())
    {
        //Vector sizes don't match, throw exception
        std::length_error mismatchedLengthException("Vectors to subtract must be of same size!\n");
        throw mismatchedLengthException;
    }

    for (unsigned int i = 0; i < this->elements.size(); i++)
    {
        this->elements[i] -= rhs.elements[i];
    }
    return *this;
}

/*
 *  Compound assignment multiplication by scalar
 * 
 * Multiplying a vector by a scalar is something we do *all* the time.
 * So, *= to the rescue!
 * 
 * Note that dot product and cross product are defined separately and only 
 * for the cases of interest. Again, this is vector *= scalar.
 */
genericVector& genericVector::operator*=(const double rhs)
{
    for (unsigned int i = 0; i < this -> elements.size(); i++)
    {
        this->elements[i] *= rhs;
    }
    return *this;
}

/*
 *  Compound assignment division by scalar
 * 
 * Multiplying a vector by the inverse of a scalar would be enough, 
 * but this is so trivial we might as well implement it for the sake
 * of completeness 
 */
genericVector& genericVector::operator/=(const double rhs)
{
    for (unsigned int i = 0; i < this -> elements.size(); i++)
    {
        this->elements[i] /= rhs;
    }
    return *this;
}

/*
 *  genericVector sum
 * 
 * As is usual, uses compound operator for heavy lifting
 * 
 * Declared   because it really should be
 */
  genericVector operator+(genericVector lhs, const genericVector& rhs)
{
    lhs += rhs;
    return lhs;
}

/*
 *  genericVector subtraction
 * 
 * As is usual, uses compound operator for heavy lifting
 * 
 * Declared   because it really should be
 */
  genericVector operator-(genericVector lhs, const genericVector& rhs)
{
    lhs -= rhs;
    return lhs;
}

/*
 *  genericVector multiplication with scalar
 * 
 * As is usual, uses compound operator for heavy lifting. Two versions
 * exist, because this operation is commutative.
 * 
 * Declared   because it really should be
 */
  genericVector operator*(genericVector lhs, double rhs)
{
    return lhs *= rhs;
}
// Reverse order of parameters to satisfy commutativity
  genericVector operator*(double lhs, genericVector rhs)
{
    return rhs *= lhs;
}

/*
 *  genericVector division with scalar
 * 
 * As is usual, uses compound operator for heavy lifting. Only one
 * version exists because division by vector is not defined.
 * 
 * Declared   because it really should be
 */
  genericVector operator/(genericVector lhs, const double rhs)
{
    return lhs /= rhs;
}




/*
 *
 *          ***** Miscellaneous functions *****
 * 
 */

/*
 *  Simple, safe function to get an element from the vector
 * 
 * Declared  
 */
  double genericVector::getVal(unsigned int position) const
{
    if (position >= this->elements.size())
    {
        //Requested element does not exist, throw exception
        std::out_of_range invalidSubscriptException("Requested vector element is out of bounds!\n");
        throw invalidSubscriptException;
    }
    return this->elements[position];
}

/*
 *  Simple, safe function to set an element from the vector
 * 
 * Declared  
 */
  void genericVector::setVal(unsigned int position, double value)
{
    if (position >= this->elements.size())
    {
        //Requested element does not exist, throw exception
        std::out_of_range invalidSubscriptException("Requested vector element is out of bounds!\n");
        throw invalidSubscriptException;
    }
    this->elements[position] = value;
}

/*
 *  Pretty-print a linear vector
 */
void genericVector::print(std::ostream& dest)
{
    //dest.width(8);
    //dest.precision(9);

    dest << "[  ";
    for (auto i : this -> elements)
    {
        dest << i << "  ";
    }
    dest << "]" << std::endl;
}





/*
 *  ---=== MCISvector function definitions ===---
 *
 *          ***** Constructors *****
 * 
 */ 

/*
 *  Default constructor
 * 
 * This one just creates an appropriately-sized vector by calling the parent's
 * constructor.
 */
MCISvector::MCISvector() : genericVector(3)
{}

/*
 *  Initial length constructor
 * 
 *  Constructs the elements vector with initial size vectorLen
 */
MCISvector::MCISvector(double a, double b, double c) : genericVector(3)
{
    elements[0] = a;
    elements[1] = b;
    elements[2] = c;
}

/*
 *  Vector initializer constructor
 */
MCISvector::MCISvector(const std::vector<double>& scalars) : genericVector(3)
{
    if (scalars.size() != 3)
    {
        //Throw an exception since we have the wrong size
        std::length_error mismatchedLengthException("MCISvectors must be of size 3!\n");
        throw mismatchedLengthException;
    }
    this->elements = scalars;
}

/*
 *  Vector initializer constructor, move version
 */
MCISvector::MCISvector(std::vector<double>&& scalars) : genericVector(3)
{
    if (scalars.size() != 3)
    {
        //Throw an exception since we have the wrong size
        std::length_error mismatchedLengthException("MCISvectors must be of size 3!\n");
        throw mismatchedLengthException;
    }
    this->elements.swap(scalars);
}

/* 
 *  Copy constructor
 */
MCISvector::MCISvector(const MCISvector& toBeCopied) : genericVector(3)
{
    this->elements = toBeCopied.elements;
}

/* 
 *  Move constructor
 */
MCISvector::MCISvector(MCISvector&& toBeCopied) : genericVector(3)
{
    this->elements.swap(toBeCopied.elements);
}

/*
 *  Convenient all-at-once assignment
 */
void MCISvector::assign(double a, double b, double c)
{
    elements[0] = a;
    elements[1] = b;
    elements[2] = c;
}

/*
 *  Calculate the dot product of two vectors of length 3
 * 
 *  This is coded extremely explicitly to make it easier for the
 * compiler to optimize into SIMD, whenever possible.
 * 
 * Also, it's a pre-unrolled loop. Sure, it takes up more space in the binary,
 * but who cares? Even on a Pentium 4 this is going to be beneficial. 
 * If there's one thing that NetBurst does well, it's doing a long stream of
 * operations without branches.
 * 
 * Doubly so for modern CPUs.
 */
double MCISvector::dotProduct(const MCISvector& aVector, const MCISvector& bVector)
{
    double output;

    output =    aVector.elements[0]*bVector.elements[0] + 
                aVector.elements[1]*bVector.elements[1] +
                aVector.elements[2]*bVector.elements[2];
    
    return output;            
}


/*
 *  Calculate the cross product of two vectors of length 3
 */
MCISvector MCISvector::crossProduct(const MCISvector& aVector, const MCISvector& bVector)
{
    std::vector<double> result (3);
    result[0] = aVector.elements[1]*bVector.elements[2] - aVector.elements[2]*bVector.elements[1];
    result[1] = aVector.elements[2]*bVector.elements[0] - aVector.elements[0]*bVector.elements[2];
    result[2] = aVector.elements[0]*bVector.elements[1] - aVector.elements[1]*bVector.elements[0];

    MCISvector resOut(result);

    //return std::move(resOut);
    return resOut;
}

/*
 *  Apply a set of three scalar gains to the vector
 */
void MCISvector::applyScalarGains(double a, double b, double c)
{
    elements[0] *= a;
    elements[1] *= b;
    elements[2] *= c;

}

/*
 *          ***** Operator Overloads *****
 */
MCISvector& MCISvector::operator=(const MCISvector& rhs)
{
     if (this != &rhs)
    {
        elements = rhs.elements;
    }
    return *this;
}
MCISvector& MCISvector::operator=(MCISvector&& rhs)
{
    if (this != &rhs)
    {
        elements.swap(rhs.elements);
    }
    return *this;
}
/* 
 * These all call their parent operator functions. Behavior is identical,
 * but this is a required formality.
 */
  bool MCISvector::operator==(const MCISvector& rhs)
{
    return genericVector::operator==(rhs);
}
  bool MCISvector::operator!=(const MCISvector& rhs)
{
    return genericVector::operator!=(rhs);
}

  MCISvector& MCISvector::operator+=(const MCISvector& rhs)
{
    genericVector::operator+=(rhs);
    return *this;
}
  MCISvector& MCISvector::operator-=(const MCISvector& rhs)
{
    genericVector::operator-=(rhs);
    return *this;
}
  MCISvector& MCISvector::operator*=(double rhs)
{
    genericVector::operator*=(rhs);
    return *this;
}
  MCISvector& MCISvector::operator/=(double rhs)
{
    genericVector::operator/=(rhs);
    return *this;
}

/*
 *  For binary operators, we call the compound operators
 * for MCISvector
 */
MCISvector operator+(MCISvector lhs, const MCISvector& rhs)
{
    return lhs += rhs;
}
MCISvector operator-(MCISvector lhs, const MCISvector& rhs)
{
    return lhs -= rhs;
}
MCISvector operator*(MCISvector lhs, double rhs)
{
    return lhs *= rhs;
}
MCISvector operator*(double lhs, MCISvector rhs)
{
    return rhs *= lhs;
}
MCISvector operator/(MCISvector lhs, double rhs)
{
    return lhs /= rhs;
}


/*
 *  ---=== MCISmatrix function definitions ===---
 *
 *          ***** Constructors *****
 * 
 */ 
/*
 *  Default constructor
 * 
 * Constructs the elements vector with size 9
 */
MCISmatrix::MCISmatrix() : genericVector(9)
{}

/*
 *  Convenient constructor
 * 
 *  Constructs the elements vector with initial size 9 and assigns
 * nine doubles
 */
MCISmatrix::MCISmatrix( double a, double b, double c,
                        double d, double e, double f,
                        double g, double h, double i) : genericVector(9)
{
    this -> elements[0] = a;
    this -> elements[1] = b;
    this -> elements[2] = c;

    this -> elements[3] = d;
    this -> elements[4] = e;
    this -> elements[5] = f;

    this -> elements[6] = g;
    this -> elements[7] = h;
    this -> elements[8] = i;
}

/*
 *  std::vector copy constructor
 */
MCISmatrix::MCISmatrix(const std::vector<double>& scalars) : genericVector(9)
{
    if (scalars.size() != 9)
    {
        //Throw an exception since we have the wrong size
        std::length_error mismatchedLengthException("MCISmatrix must have 9 elements!\n");
        throw mismatchedLengthException;
    }
    this -> elements = scalars;
}

/*
 *  std::vector move constructor
 */
MCISmatrix::MCISmatrix(std::vector<double>&& scalars) : genericVector(9)
{
    if (scalars.size() != 9)
    {
        //Throw an exception since we have the wrong size
        std::length_error mismatchedLengthException("MCISmatrix must have 9 elements!\n");
        throw mismatchedLengthException;
    }
    this -> elements.swap(scalars);
}

/*
 *  Copy constructor
 */
MCISmatrix::MCISmatrix(const MCISmatrix& toBeCopied) : genericVector(toBeCopied.elements)
{}

/*
 *  Move constructor
 */
MCISmatrix::MCISmatrix(MCISmatrix&& toBeMoved) : genericVector(9)
{
    this -> elements.swap(toBeMoved.elements);
}



/*
 *          ***** Operator OVerloads *****
 */
  MCISmatrix& MCISmatrix::operator=(const MCISmatrix& rhs)
{
     if (this != &rhs)
    {
        elements = rhs.elements;
    }
    return *this;
}
  MCISmatrix& MCISmatrix::operator=(MCISmatrix&& rhs)
{
    if (this != &rhs)
    {
        elements.swap(rhs.elements);
    }
    return *this;
}
/* 
 * These all call their parent operator functions. Behavior is identical,
 * but this is a required formality.
 */
  bool MCISmatrix::operator==(const MCISmatrix& rhs)
{
    return genericVector::operator==(rhs);
}
  bool MCISmatrix::operator!=(const MCISmatrix& rhs)
{
    return genericVector::operator!=(rhs);
}

  MCISmatrix& MCISmatrix::operator+=(const MCISmatrix& rhs)
{
    genericVector::operator+=(rhs);
    return *this;
}
  MCISmatrix& MCISmatrix::operator-=(const MCISmatrix& rhs)
{
    genericVector::operator-=(rhs);
    return *this;
}
  MCISmatrix& MCISmatrix::operator*=(double rhs)
{
    genericVector::operator*=(rhs);
    return *this;
}
  MCISmatrix& MCISmatrix::operator/=(double rhs)
{
    genericVector::operator/=(rhs);
    return *this;
}

/*
 *  For binary operators, we call the compound operators
 * for MCISmatrix
 */
  MCISmatrix operator+(MCISmatrix lhs, const MCISmatrix& rhs)
{
    return lhs += rhs;
}
  MCISmatrix operator-(MCISmatrix lhs, const MCISmatrix& rhs)
{
    return lhs -= rhs;
}
  MCISmatrix operator*(MCISmatrix lhs, double rhs)
{
    return lhs *= rhs;
}
  MCISmatrix operator*(double lhs, MCISmatrix rhs)
{
    return rhs *= lhs;
}
  MCISmatrix operator/(MCISmatrix lhs, double rhs)
{
    return lhs /= rhs;
}

/*
 *  Multiplying a  3x3 matrix with a 3x1 column vector is done by calling
 * MCISmatrix::rightMultiplyVector()
 */
MCISvector operator*(const MCISmatrix& lhs, const MCISvector& rhs)
{
    return lhs.rightMultiplyVector(rhs);
}


/*
 *          ***** Assignment and retrieval *****
 */
/*
 *  Assign all values at once
 * 
 * Useful for prototyping and initialization
 * 
 * The matrix is defined with the following indices:
 * 
 * | 0 1 2 |
 * | 3 4 5 |
 * | 6 7 8 |
 */
  void MCISmatrix::assign(double a, double b, double c,
                               double d, double e, double f,
                               double g, double h, double i)
{
    this -> elements[0] = a;
    this -> elements[1] = b;
    this -> elements[2] = c;

    this -> elements[3] = d;
    this -> elements[4] = e;
    this -> elements[5] = f;

    this -> elements[6] = g;
    this -> elements[7] = h;
    this -> elements[8] = i;
}

/*
 *  Get an element from the storage vector using matrix notation
 * (row, column)
 * 
 * Make sure it's not out-of-bounds
 */
double MCISmatrix::getMatrixElement(unsigned int row, unsigned int column) const
{
    if (row >= 3 || column >= 3)
    {
        //Requested element does not exist, throw exception
        std::out_of_range invalidSubscriptException("Requested vector element is out of bounds!\n");
        throw invalidSubscriptException;
    }
    return this -> elements[row * 3 + column];
}

/*
 *  Set an element from the storage vector using matrix notation
 * (row, column)
 * 
 * Make sure it's not out-of-bounds
 */
void MCISmatrix::setMatrixElement(unsigned int row, unsigned int column, double value)
{
    if (row >= 3 || column >= 3)
    {
        //Requested element does not exist, throw exception
        std::out_of_range invalidSubscriptException("Requested vector element is out of bounds!\n");
        throw invalidSubscriptException;
    }
    this -> elements[row * 3 + column] = value;
}

/*
 *  Pretty-print a matrix
 * 
 * This function overrides genericVector::print
 */
void MCISmatrix::print(std::ostream& dest)
{
    //dest.width(8);
    //dest.precision(9);

    for (int i = 0; i < 3; i++)
    {
        dest << "|  ";
        for (int j = 0; j <3; j++)
        {
            dest << this -> elements[i*3 + j] << "  ";
        }
        dest << "|" << std::endl;
    }
}





/* 
 *          ***** Matrix operations *****
 */
/*
 *  Right-multiply a 3x3 matrix with a 3x1 vector:
 * 
 *  | a b c || j |   | m |
 *  | d e f || k | = | n |
 *  | g h i || l |   | o |
 */
MCISvector MCISmatrix::rightMultiplyVector(const MCISvector& vec) const
{
    MCISvector result;
    double res;

    for(int rowIt = 0; rowIt < 3; rowIt++)
    {
        res = 0;
        for (int colIt = 0; colIt < 3; colIt++)
        {
            res += (this->elements[rowIt*3 + colIt])*vec.getVal(colIt);
        }
        result.setVal(rowIt, res);
    }
    return result;
}

/*
 *  Tranpose the current matrix
 * 
 * The object pointed to by this is *OVERWRITTEN*
 * 
 * This is fine for MCIS because we never really need to reuse the matrix later,
 * so we can save on some allocations. Allocations are evil.
 * 
 * If needed in the future, a second function can backup the original
 * matrix before transposing, then swap them, then return the original one
 */
void MCISmatrix::transpose()
{
    double temp;

    temp = this -> elements[1];                 // temp holds b
    this  -> elements[1] = this -> elements[3]; // b now holds d
    this -> elements[3] = temp;                 // and d holds b

    temp = this -> elements[2];                 // temp holds c
    this  -> elements[2] = this -> elements[6]; // c now holds g
    this -> elements[6] = temp;                 // and g holds c

    temp = this -> elements[5];                 // temp holds f
    this  -> elements[5] = this -> elements[7]; // f now holds h
    this -> elements[7] = temp;                 // and h holds f
}

/*
*  Calculate the inverse Direction Cosines Matrix for ZYX rotation
* 
* The reader of the above description is naturally left with a number 
* of questions. Let's address them:
* 
* Why do we need the inverse DCM?
* - The DCM allows us to rotate our frame of reference to body axes
*   from pseudo-inertial (Earth-fixed) axes. In our case, we actually 
*   want to convert our body axes to pesudo-inertial axes, since the 
*   Motion Base receives commands in this frame of reference.
* 
* You're not actually inverting the matrix!
* - DCMs are orthogonal matrices. Thus, inv(A)==A'.
*   To make things easier for ourselves, we also pre-transpose
*   the matrix by defining its transpose instead of the original matrix.
*   There's no point in doing this calculation thousands of times, 
*   needlessly.
* 
* Why ZYX and not XYZ or another rotation?
* - Roll, Pitch and Yaw are defined as a ZYX rotation, as this makes
*   a zero rotation correspond to "level" attitude.
*/
void MCISmatrix::euler2DCM_ZYX_inv(const MCISvector& eulerAngles)
{
    /*
        *  We need the sine and cosine of every element of eulerAngles,
        *  and we need them repeatedly. We'll pre-calculate these first,
        *  then do the rest of the work in a vectorization-friendly manner. 
        */

    double sPhi, sTheta, sPsi, cPhi, cTheta, cPsi;

    sPhi    = sin(eulerAngles.getVal(0));
    sTheta  = sin(eulerAngles.getVal(1));
    sPsi    = sin(eulerAngles.getVal(2));

    cPhi    = cos(eulerAngles.getVal(0));
    cTheta  = cos(eulerAngles.getVal(1));
    cPsi    = cos(eulerAngles.getVal(2));

    /*
        *  Now we can assign the matrix elements
        * 
        * Remember that we're in Row-Column order:
        * 
        * | 0 1 2 |
        * | 3 4 5 |
        * | 6 7 8 |
        * 
        * Also, don't forget that we're assigning the transpose.
        */
    this -> elements[0] = cTheta * cPsi;
    this -> elements[1] = sPhi*sTheta*cPsi - cPhi*sPsi; 
    this -> elements[2] = cPhi*sTheta*cPsi + sPhi*sPsi;

    this -> elements[3] = cTheta*sPsi;
    this -> elements[4] = sPhi*sTheta*sPsi + cPhi*cPsi;
    this -> elements[5] = cPhi*sTheta*sPsi - sPhi*cPsi;

    this -> elements[6] = - sTheta;
    this -> elements[7] = sPhi * cTheta;
    this -> elements[8] = cPhi * cTheta;
}

/*
     *
     * Calculate the transformation matrix from body angular velocities to
     * Euler angle rates. 
     * 
     */
    void MCISmatrix::pqr2eulerRates(const MCISvector& eulerAngles)
    {
        /*
         *  We'll need these values repeatedly, so we'll precalculate them
         */
        double sPhi, cPhi, tanTheta, secTheta;

        sPhi    = sin(eulerAngles.getVal(0));
        cPhi    = cos(eulerAngles.getVal(0));

        tanTheta = tan(eulerAngles.getVal(1));
        secTheta = 1 / cos(eulerAngles.getVal(1));

        /*
        *  Now we can assign the matrix elements
        * 
        * Remember that we're in Row-Column order:
        * 
        * | 0 1 2 |
        * | 3 4 5 |
        * | 6 7 8 |
        * 
        * Also, don't forget that we're assigning the transpose.
        */
        this -> elements[0] = 1;
        this -> elements[1] = sPhi * tanTheta; 
        this -> elements[2] = cPhi * tanTheta;

        this -> elements[3] =   0;
        this -> elements[4] =   cPhi;
        this -> elements[5] = - sPhi;

        this -> elements[6] = 0;
        this -> elements[7] = sPhi * secTheta;
        this -> elements[8] = cPhi * secTheta;
    }





/*
 *  
 *          ---=== class saturation ===---
 * 
 */
/*
 *  Constructor. Takes init settings.
 * 
 * We don't do default constructions, because defaults would rarely
 * make any sense. This forces users to think before using a limit.
 * It's a start, at least.
 */
saturation::saturation(double limSetting, double initOutput)
{
    limit   = limSetting;
    output  = initOutput;
}

/*
 *  Next sample runs one iteration of the rate limit.
 * 
 * If the output is within the limits, it's passed through.
 * If it isn't, it gets clamped down to the corresponding positive or 
 * negative limit, depending on sign.
 */
double saturation::nextSample(double input)
{   
    if (input > limit)
    {
        output = limit;
    }
    else if (input < -limit)
    {
        output = -limit;
    }
    else
    {
        output = input;
    }
    return output;
}

/*
 *  Set the limit member after construction.
 * 
 * This function is useful for adaptive filtering, but must be used with caution.
 * 
 * Thread safety must be ensured at a higher level!!!
 */
void saturation::setLimit(double newLim)
{
    limit = newLim;
}




/*
 *
 *          ---=== class saturation ===---
 * 
 */

/*
 *  This constructor is a formality.
 * 
 * It calls the parent constructor for heavy lifting.
 */
rateLimit::rateLimit(double limSetting, double initOutput) : saturation(limSetting, initOutput) {}

/*
 *  nextSample calculates the next iteration.
 * 
 * This is where the previous output is used. If the difference between 
 * the new input and the old output is greater in magnitude than the set limit,
 * the limit is added/subtracted to the output and the input discarded.
 * 
 * If the limit is not reached, the output is passed through.
 */
double rateLimit::nextSample(double input)
{
    double inputRate = input - output; //x(n) - x(n-1)
    double absRate = fabs(inputRate);

    if (absRate > limit)
    {
        if (inputRate < 0)
        {
            output -= limit;
        }
        else
        {
            output += limit;
        }
    }
    else
    {
        output = input;
    }
    
    /*if (abs(inputRate) > limit && inputRate < 0)
    {
        output -= limit;
    }
    else if (abs(inputRate) > limit && inputRate > 0)
    {
        output += limit;
    }
    else
    {
        output = input;
    }*/
    
    return output;
}

/*
 *  This new output is never actually output, it's only used as the reference 
 *  when calculating the rate for the next iteration
 */
void rateLimit::overrideOutput(double newOutput)
{
    output = newOutput;
}



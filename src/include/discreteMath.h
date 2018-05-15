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

#pragma once

#include <mutex>
#include <vector>
#include <iostream> 
#include "MCIS_config.h"

#define gravity 9.81


/*
 * The discreteFilt class implemets a discrete-time, 2nd order direct form II filter.
 * 
 *
 */
class discreteFilt2ndOrder
{
    private:

    double delays[3];
    double aGains[3];
    double bGains[3];
    
    double currOutput;

    
    public:
    
    /*
     * Basic constructor. Sets the filter up for operation
     * 
     * Filter order must be one lower than the lengths of aGainIn and bGainIn
     */
    discreteFilt2ndOrder(const discreteBiquadSectionParams& config);
    discreteFilt2ndOrder(const std::vector<double>& bGainIn, const std::vector<double>& aGainIn);
    
    //Change filter parameters.
    void setParams(const discreteBiquadSectionParams&);
    void setParams(const std::vector<double>& aGainIn, const std::vector<double>& bGainIn);
    //Reset filter internal state to zero
    void resetState();
    //Set a specific state
    void setState(const std::vector<double>& newState);

    // Run filter for one sample and output the new output
    double nextSample(double newInput);
    
};

/*
 *  The genericVector class implements the very basics needed for
 * vector math: storage, read/write access, vector addition and subtraction
 * and multiplication/division by a scalar, as well as some basic constructors.
 * 
 * It is not meant for use in MCIS, rather it is the parent class for 
 * MCISvector and MCISmatrix, both of which have strictly-defined sizes 
 * (1x3 or 3x1 and 3x3, respectively).
 * 
 * Notably absent are the dot and cross products, as well as any sort of matrix 
 * operations. These are defined in the subclasses of interest.
 * 
 * For storage, std::vector is used.
 */
class genericVector
{
    protected:
    
    //The actual storage
    std::vector<double> elements;

    // Protected constructor for initial length
    //  Used by subclasses with known sizes
    genericVector(const unsigned int vectorLen);
    
    
    public:
    
    genericVector(genericVector&& toBeMoved);  //Move constructor
    genericVector(const genericVector& toBeCopied);  //Copy constructor
    
    genericVector(const std::vector<double>& scalars); //New generic-size vector
    genericVector(std::vector<double>&& scalars); //New generic-size vector, move version
    //~genericVector();   //Destructor is trivial
    
    //Copy and move assignment operators
    genericVector& operator=(const genericVector& rhs);
    genericVector& operator=(genericVector&& rhs);
    
    //Equality and inequality operators
    bool operator==(const genericVector& rhs);
    bool operator!=(const genericVector& rhs);
    
    //Compound arithmetic assignment operators
    genericVector& operator+=(const genericVector& rhs);
    genericVector& operator-=(const genericVector& rhs);
    /*
     *  Sum and subtraction can introduce a subtle bug if used incorrectly:
     * They will not fail when trying to sum a 3x3 matrix and a 1x9 vector, 
     * for instance. Since we have no use for 1x9 vectors in MCIS, we
     * don't care about this scenario.
     */
    genericVector& operator*=(double rhs);
    genericVector& operator/=(double rhs);

    //Regular ol' binary arithmetic operators
    friend genericVector operator+(genericVector lhs, const genericVector& rhs);
    friend genericVector operator-(genericVector lhs, const genericVector& rhs);
    friend genericVector operator*(genericVector lhs, double rhs);
    friend genericVector operator/(genericVector lhs, double rhs);
 
    //Bounds-safe getter and setter for individual values    
    double  getVal(unsigned int position) const;
    void    setVal(unsigned int position, double value);

    //Pretty-print a linear vector
    void print(std::ostream& dest);
};

/*
 *  All these declarations correspond to previously-declared friends of -------
 * genericVector. Skip to the next comment.
 */
genericVector operator+(const genericVector& rhs);
genericVector operator-(const genericVector& rhs);

genericVector operator*(genericVector lhs, const double rhs);
genericVector operator/(genericVector lhs, const double rhs);
genericVector operator*(double lhs, const genericVector& rhs);

/*
 *  Friends of genericVector end here. Skip to here. --------------------------
 */



/*
 *  MCISvector defines a standard three-dimension vector, useful for physics
 * calculations in three-dimensional space.
 * 
 * Length is guaranteed to be 3
 */
class MCISvector: public genericVector
{
    public:

    //Default constructor
    MCISvector();

    //Convenient constructor
    MCISvector(double a, double b, double c);    
    
    //Copy and move constructors for std::vector
    MCISvector(const std::vector<double>& scalars);
    MCISvector(std::vector<double>&& scalars); 

    //Copy and move constructors
    MCISvector(const MCISvector& toBeCopied);
    MCISvector(MCISvector&& toBeMoved);

    //Operator overloads

    //Copy and move assignment operators
    MCISvector& operator=(const MCISvector& rhs);
    MCISvector& operator=(MCISvector&& rhs);
    
    //Equality and inequality operators
    bool operator==(const MCISvector& rhs);
    bool operator!=(const MCISvector& rhs);
    
    //Compound arithmetic assignment operators
    MCISvector& operator+=(const MCISvector& rhs);
    MCISvector& operator-=(const MCISvector& rhs);
    MCISvector& operator*=(double rhs);
    MCISvector& operator/=(double rhs);

    //Regular ol' binary arithmetic operators
    friend MCISvector operator+(MCISvector lhs, const MCISvector& rhs);
    friend MCISvector operator-(MCISvector lhs, const MCISvector& rhs);
    friend MCISvector operator*(MCISvector lhs, double rhs);
    friend MCISvector operator/(MCISvector lhs, double rhs);


    void assign(double a, double b, double c);  //Convenient assignment
                                                //for existing vectors.

    /*
     *  The main reason why dot product is here and not in genericVector is
     * so that nobody tries to do so on a matrix, clobbering things.
     * 
     * You could argue that this is poor design and there should be two 
     * subclasses of genericVector: linearVector and matrixVector, the latter
     * of which keeping track of matrix dimensions - or even do so in 
     * genericVector.
     * 
     * This is a good point, but the added complexity is not needed for MCIS
     * and this is not a MATLAB replacement.
     */
    static double       dotProduct(const MCISvector& aVector, const MCISvector& bVector);

    //Cross product is only defined for 1x3 or 3x1 vectors, so it goes here.
    static MCISvector   crossProduct(const MCISvector& aVector, const MCISvector& bVector);

    //Apply scalar gains to each element of the vector
    void applyScalarGains(double a, double b, double c);   
};

MCISvector operator+(MCISvector lhs, const MCISvector& rhs);
MCISvector operator-(MCISvector lhs, const MCISvector& rhs);
MCISvector operator*(MCISvector lhs, double rhs);
MCISvector operator*(double lhs, MCISvector rhs);
MCISvector operator/(MCISvector lhs, double rhs);

/*
 *  MCISmatrix defines a standard 3x3 matrix, useful for transformations on
 * 1x3 vectors common in physics.
 * 
 * Length is guaranteed to be 9.
 * 
 * The matrix is defined with the following indices:
 * 
 * | 0 1 2 |
 * | 3 4 5 |
 * | 6 7 8 |
 * 
 * This class defines a few important functions:
 *      - det: Returns the determinant of the matrix
 *      - getInverse: returns the inverse matrix
 * 
 *      - multiplication with vector (3x1 on the right)
 *      - multiplication with matrix (3x3 on the right)
 *  
 *      - getMAtrixElement: get an element in matrix notation
 * 
 */
class MCISmatrix: public genericVector
{
    public:
    //Default constructor
    MCISmatrix();

    //Convenient constructor
    MCISmatrix( double a, double b, double c,
                double d, double e, double f,
                double g, double h, double i);

    //Copy and move constructors for std::vector
    MCISmatrix(const std::vector<double>& scalars);
    MCISmatrix(std::vector<double>&& scalars);

    //Copy and move constructors
    MCISmatrix(const MCISmatrix& toBeCopied);
    MCISmatrix(MCISmatrix&& toBeMoved);

    //Convenient all-at-once assignment
    void assign(double a, double b, double c,
                double d, double e, double f,
                double g, double h, double i);

    //Operator overloads

    //Copy and move assignment operators
    MCISmatrix& operator=(const MCISmatrix& rhs);
    MCISmatrix& operator=(MCISmatrix&& rhs);
    
    //Equality and inequality operators
    bool operator==(const MCISmatrix& rhs);
    bool operator!=(const MCISmatrix& rhs);
    
    //Compound arithmetic assignment operators
    MCISmatrix& operator+=(const MCISmatrix& rhs);
    MCISmatrix& operator-=(const MCISmatrix& rhs);
    MCISmatrix& operator*=(double rhs);
    MCISmatrix& operator/=(double rhs);

    //Regular ol' binary arithmetic operators
    friend MCISmatrix operator+(MCISmatrix lhs, const MCISmatrix& rhs);
    friend MCISmatrix operator-(MCISmatrix lhs, const MCISmatrix& rhs);
    friend MCISmatrix operator*(MCISmatrix lhs, double rhs);
    friend MCISmatrix operator/(MCISmatrix lhs, double rhs);

    //operator* for A*b = c
    friend MCISvector operator*(const MCISmatrix& lhs, const MCISvector& rhs);


    /*  
     * Getter and setter in matrix notation
     * The linear version from the base class is also usable, but this
     * one is more convenient. 
     */
    double getMatrixElement(unsigned int row, unsigned int column) const;
    void   setMatrixElement(unsigned int row, unsigned int column, double value);   


    /* 
     *  Matrix operations
     * 
     * The following operations were considered but ultimately rejected
     * for MCIS, due to them not being needed.
     * 
     * They are listed here to give a better understanding of the
     * limited set of available operations.
     * 
     * 1: Right-multiply 3x3 matrix with 3x3 matrix. Result is 3x3 matrix.
     * MCISmatrix rightMultiplyMatrix(const MCISmatrix& matrix);
     * We never actually need to multiply matrices, just column vectors.
     * 
     * 2: Calculate the determinant of the matrix
     * double det();
     * The determinant is mostly useful for matrix inversion in MCIS. However,
     * we sidestep around matrix inversion because Direction Cosine Matrices
     * are orthogonal, and thus A^-1 == A'
     * 
     * 3: Calculate the inverse of the matrix
     * MCISmatrix getInverse();
     * We sidestep around matrix inversion because Direction Cosine Matrices
     * are orthogonal, and thus A^-1 == A'. Transposition is much cheaper.
     */
    
    //Pretty-print a matrix. Overrides the base class printer
    void print(std::ostream& dest); 
    
    //Right-multiply 3x3 matrix with 3x1 vector. Result is 3x1 vector.
    MCISvector rightMultiplyVector(const MCISvector& vec) const;

    
    /*  Transpose this matrix
     *  This OVERWRITES the current matrix
     * 
     * Note the lack of const designation on this function.
     * 
     * Again, it does A = A'. It is NOT read-only.
     */
    void transpose();

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
    void euler2DCM_ZYX_inv(const MCISvector& eulerAngles);

    /*
     *
     * Calculate the transformation matrix from body angular velocities to
     * Euler angle rates. 
     * 
     */
    void pqr2eulerRates(const MCISvector& eulerAngles);

    

};

MCISmatrix operator+(MCISmatrix lhs, const MCISmatrix& rhs);
MCISmatrix operator-(MCISmatrix lhs, const MCISmatrix& rhs);
MCISmatrix operator*(MCISmatrix lhs, double rhs);
MCISmatrix operator*(double lhs, MCISmatrix rhs);
MCISmatrix operator/(MCISmatrix lhs, double rhs);

//operator* for A*b = c
MCISvector operator*(const MCISmatrix& lhs, const MCISvector& rhs);


/*
 *  The saturation class implements a simple saturation.
 * 
 * Values are not allowed to exceed the magnitude of the limit member.
 * In other words, limits are imposed as 0+-limit, so +limit and -limit
 */
class saturation
{
    protected:
    
    //The limit to set
    double  limit;
    //Output storage. Not very useful here, but needed in rateLimit
    double  output;
    
    
    public:
    
    //Constructors
    saturation(){};
    saturation(double limSetting, double initOutput);
    
    //Do one iteration using the input parameter as input and return the output
    double nextSample(double input);
    
    //Change the limit after construction
    //Potentially dangerous, don't use willy-nilly.
    void setLimit(double newLim);
    
};

/*
 *  The rate limit class inherits from saturation.
 * This will tickle your OCD, but is fine here. Actual functionality is 
 * provided by an overriden nextSample function
 */
class rateLimit: public saturation
{
    public:

    //Formality, calls parent constructor
    rateLimit(double limSetting, double initOutput);
    
    //Overriden to do a rate limit using the stored previous output
    double nextSample(double input);

    //Allow the output to be overriden. Useful when an instantaneous change 
    //is needed. When you'd need it is a good question.
    void overrideOutput(double newOutput);
    
};

/*
 *  vectorRateLimit
 * 
 * Apply a scalar rate limit to every element of the vector
 * 
 * This class could be refactored for genericVector, but MCISvector is fine for us
 */
class vectorRateLimit
{
    private:
    //Scalar rate limits
    rateLimit lim0, lim1, lim2;

    public:
    //Constructor
    vectorRateLimit(double rateLimit, const MCISvector& initOutput);

    void nextSample(MCISvector& input);
    void overrideOutput(const MCISvector& newOutput);
};


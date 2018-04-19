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

//Simple test for the limits classes...
//Then it turned into a Smorgasborb of tests for discreteMath.h

#include <cstdlib>
#include <unistd.h>
#include <iostream>
#include <ostream>
#include <vector>
#include "discreteMath.h"

int main(void)
{
    std::cout << "Testing limits:" << std::endl;

    double inputList[] = {0, 1, 1, 2, 3, 3.5, 5, 10, -10, -15, 20, 15, 9, 7, -10, -30, -100, 0, 20, 10};
    
    double limOut[20];
    double rateLimOut[20];
    
    std::cout << "Outputs:" << std::endl;
    
    saturation limiter(10.0, 0.0);
    rateLimit  rateLimiter(5.0, 0.0);
    
    for (int j = 0; j < 20; j++)
    {
        limOut[j] = limiter.nextSample(inputList[j]);
        rateLimOut[j] = rateLimiter.nextSample(inputList[j]);
        
        std::cout << inputList[j] << "  |  " << limOut[j] << "  |  " << rateLimOut[j] << std::endl;
    }
    
    std::cout << "Limits tested" << std::endl << std::endl << std::endl << std::endl << std::endl;

    std::cout << "Testing MCISvector definitions: Empty vector, 123, 987, 1.5 24.2 1.347" << std::endl;

    MCISvector emptyVector;
    MCISvector oneTwoThree(1,2,3);

    std::vector<double> toBeKept{9, 8, 7};
    std::vector<double> toBeDestroyed{1.5, 24.2, 1.347};

    MCISvector nineEightSeven(toBeKept);
    MCISvector doubleGoodness(std::move(toBeDestroyed));

    emptyVector.print(std::cout);
    oneTwoThree.print(std::cout);
    nineEightSeven.print(std::cout);
    doubleGoodness.print(std::cout);
    


    std::cout <<  std::endl << "Testing copy and move constructors." << std::endl;
    MCISvector doubleNew(doubleGoodness);
    std::cout << "Copy-constructed: ";
    doubleNew.print(std::cout);
    std::cout << "Original vector:  ";
    doubleGoodness.print(std::cout);

    std::cout << "Move destination: ";
    MCISvector doubleReplacement(std::move(doubleGoodness));
    doubleReplacement.print(std::cout);
    std::cout << "Original vector:  ";
    doubleGoodness.print(std::cout);



    std::cout <<  std::endl << "Testing vector assignment. Move assigning 123 to zero vector:" << std::endl;
    emptyVector = std::move(oneTwoThree);
    nineEightSeven = emptyVector;

    std::cout << "Empty: ";
    emptyVector.print(std::cout);
    std::cout << "123:   ";
    oneTwoThree.print(std::cout);

    std::cout << std::endl << "Copy assigning empty to 987:" << std::endl;

    std::cout << "Empty: ";
    emptyVector.print(std::cout);
    std::cout << "123:   ";
    oneTwoThree.print(std::cout);
    std::cout << "987:   ";
    nineEightSeven.print(std::cout);



    std::cout <<  std::endl << "Testing vector equality:" << std::endl;

    bool equal;

    equal = (emptyVector == nineEightSeven);
    std::cout << "Empty == 987 [true]: " << equal << std::endl;

    equal = (emptyVector != nineEightSeven);
    std::cout << "Empty != 987 [false]: " << equal << std::endl;

    equal = (emptyVector == oneTwoThree);
    std::cout << "Empty == 123 [false]: " << equal << std::endl;

    equal = (emptyVector != oneTwoThree);
    std::cout << "Empty != 123 [true]: " << equal << std::endl;

    
    
    std::cout <<  std::endl << "Testing vector addition [A + B = C]:" << std::endl;
    
    std::vector<double> vecAlist{1.5, 15, 0.0625};
    MCISvector vecA(vecAlist);

    std::vector<double> vecBlist{0.5, -10, 0.0625};
    MCISvector vecB(vecBlist);

    MCISvector vecC{};
    

    std::cout << "Vector A: ";
    vecA.print(std::cout);

    std::cout << "Vector B: ";
    vecB.print(std::cout);

    vecC = (vecA + vecB);

    std::cout << "Vector C: ";
    vecC.print(std::cout);

    std::cout <<  std::endl << "Testing vector subtraction [A - B = C]:" << std::endl;

    std::cout << "Vector A: ";
    vecA.print(std::cout);

    std::cout << "Vector B: ";
    vecB.print(std::cout);

    vecC = (vecA - vecB);

    std::cout << "Vector C: ";
    vecC.print(std::cout);

    std::cout <<  std::endl << "Testing vector addition [A += B]:" << std::endl;
        
    std::cout << "Vector A: ";
    vecA.print(std::cout);

    std::cout << "Vector B: ";
    vecB.print(std::cout);

    vecA += vecB;

    std::cout << "Vector A: ";
    vecA.print(std::cout);

    std::cout << "Vector B: ";
    vecB.print(std::cout);


    std::cout <<  std::endl << "Testing vector subtraction [A -= B]:" << std::endl;
        
    std::cout << "Vector A: ";
    vecA.print(std::cout);

    std::cout << "Vector B: ";
    vecB.print(std::cout);

    vecA -= vecB;

    std::cout << "Vector A: ";
    vecA.print(std::cout);

    std::cout << "Vector B: ";
    vecB.print(std::cout);

    
    std::cout <<  std::endl << "Testing A = 2 * B * 2:" << std::endl;
        
    std::cout << "Vector A: ";
    vecA.print(std::cout);

    std::cout << "Vector B: ";
    vecB.print(std::cout);

    vecA = 2 * vecB * 2;

    std::cout << "Vector A: ";
    vecA.print(std::cout);

    std::cout << "Vector B: ";
    vecB.print(std::cout);


    std::cout <<  std::endl << "Testing A *= 8:" << std::endl;
        
    std::cout << "Vector A: ";
    vecA.print(std::cout);

    std::cout << "Vector B: ";
    vecB.print(std::cout);

    vecA *= 8;

    std::cout << "Vector A: ";
    vecA.print(std::cout);

    std::cout << "Vector B: ";
    vecB.print(std::cout);

    std::cout <<  std::endl << "Testing A = B / 8:" << std::endl;
        
    std::cout << "Vector A: ";
    vecA.print(std::cout);

    std::cout << "Vector B: ";
    vecB.print(std::cout);

    vecA = vecB / 8;

    std::cout << "Vector A: ";
    vecA.print(std::cout);

    std::cout << "Vector B: ";
    vecB.print(std::cout);



    std::cout <<  std::endl << "Testing A /= 0.25:" << std::endl;
        
    std::cout << "Vector A: ";
    vecA.print(std::cout);

    std::cout << "Vector B: ";
    vecB.print(std::cout);

    vecA /= 0.25;

    std::cout << "Vector A: ";
    vecA.print(std::cout);

    std::cout << "Vector B: ";
    vecB.print(std::cout);

    std::cout <<  std::endl << "Testing dot product and cross product:" << std::endl;

    vecA.assign(1, 2, 3);
    vecB.assign(5, 6, 10);

    double dotP = MCISvector::dotProduct(vecA, vecB);
    vecC = MCISvector::crossProduct(vecA, vecB);

    std::cout << "Vector A       : ";
    vecA.print(std::cout);

    std::cout << "Vector B       : ";
    vecB.print(std::cout);

    std::cout << "Dot product    : ";
    std::cout << dotP << std::endl;

    std::cout << "Cross product  : ";
    vecC.print(std::cout);



    std::cout <<  std::endl << "Testing matrix construction:" << std::endl;

    MCISmatrix testMat(1,2,3,4,5,6,7,8,9);
    testMat.print(std::cout);

    std::cout << std::endl;

    MCISmatrix testMat2(9,8,7,6,5,4,3,2,1);
    testMat2.print(std::cout);

    std::vector<double> toBeKeptM{1,2,4,8,1.6,3.2,6.4,1.28,25.6};
    std::vector<double> toBeDestroyedM{7,4.5,12,6.3,6,124,0.23,0.1,1};

    MCISmatrix matcopied{toBeKeptM};
    MCISmatrix matmoved{toBeDestroyedM};

    std::cout << "Copy-constructed from std::vector: " << std::endl;
    matcopied.print(std::cout);
    std::cout << "Move-constructed from std::vector: " << std::endl;
    matmoved.print(std::cout);

    MCISmatrix matcopied2{matcopied};
    MCISmatrix matmoved2{std::move(matmoved)};

    std::cout << "Copied matrix: " << std::endl;
    matcopied.print(std::cout);
    std::cout << "Copy-constructed from matrix: " << std::endl;
    matcopied2.print(std::cout);
    std::cout << "Moved matrix: " << std::endl;
    matmoved.print(std::cout);
    std::cout << "Move-constructed from matrix: " << std::endl;
    matmoved2.print(std::cout);


    std::cout << std::endl << "C = A - copy assignment" << std::endl;

    std::cout << "Matrix A      : " << std::endl;
    testMat.print(std::cout);

    MCISmatrix testMat3{};
    testMat3 = testMat;

    std::cout << "Matrix A      : " << std::endl;
    testMat.print(std::cout);
    std::cout << "Copy of A     : " << std::endl;
    testMat3.print(std::cout);
    
    std::cout << std::endl << "C = A - move assignment" << std::endl;

    matmoved = std::move(matmoved2);
    std::cout << "Matrix C: " << std::endl;
    matmoved.print(std::cout);
    std::cout << "Matrix A: " << std::endl;
    matmoved2.print(std::cout);




    std::cout << std::endl << "A += B" << std::endl;
    std::cout << "Matrix A: " << std::endl;
    testMat.print(std::cout);
    std::cout << "Matrix B: " << std::endl;
    testMat2.print(std::cout);
    std::cout << "Result: " << std::endl;
    testMat += testMat2;
    testMat.print(std::cout);

    std::cout << std::endl << "A -= B" << std::endl;

    testMat -= testMat2;
    testMat.print(std::cout);

    std::cout << std::endl << "A *= 0.125" << std::endl;
    std::cout << "Matrix A: " << std::endl;
    testMat.print(std::cout);
    std::cout << "Result: " << std::endl;
    testMat *= 0.125;
    testMat.print(std::cout);



    std::cout << std::endl << "A /= 0.5" << std::endl;
    std::cout << "Matrix A: " << std::endl;
    testMat.print(std::cout);
    std::cout << "Result: " << std::endl;
    testMat /= 0.5;
    testMat.print(std::cout);

    std::cout << std::endl << "A = A + B / 0.25" << std::endl;
    std::cout << "Matrix A: " << std::endl;
    testMat.print(std::cout);
    std::cout << "Matrix B: " << std::endl;
    testMat2.print(std::cout);
    std::cout << "Result: " << std::endl;
    testMat = testMat + testMat2 / 0.25;
    testMat.print(std::cout);
    std::cout << "Matrix B: " << std::endl;
    testMat2.print(std::cout);

    std::cout << std::endl << "A = A - B * 5" << std::endl;
    std::cout << "Matrix A: " << std::endl;
    testMat.print(std::cout);
    std::cout << "Matrix B: " << std::endl;
    testMat2.print(std::cout);
    std::cout << "Result: " << std::endl;
    testMat = testMat - testMat2 * 5;
    testMat.print(std::cout);
    std::cout << "Matrix B: " << std::endl;
    testMat2.print(std::cout);






    std::cout <<  std::endl << "Testing A==B:" << std::endl;
    
    equal = (testMat == testMat);
    std::cout << "A == A [true]: " << equal << std::endl;

    equal = (testMat != testMat);
    std::cout << "A != A [false]: " << equal << std::endl;

    equal = (testMat == testMat2);
    std::cout << "A == B [false]: " << equal << std::endl;

    equal = (testMat != testMat2);
    std::cout << "A != B [true]: " << equal << std::endl;


    std::cout <<  std::endl << "Testing single-element getter and setter:" << std::endl;

    std::cout << "Matrix A: " << std::endl;
    testMat.print(std::cout);

    std::cout << "Matrix A(0,0): " << testMat.getMatrixElement(0,0) << std::endl;
    std::cout << "Matrix A(0,1): " << testMat.getMatrixElement(0,1) << std::endl;
    std::cout << "Matrix A(0,2): " << testMat.getMatrixElement(0,2) << std::endl;

    std::cout << "Matrix A(1,0): " << testMat.getMatrixElement(1,0) << std::endl;
    std::cout << "Matrix A(1,1): " << testMat.getMatrixElement(1,1) << std::endl;
    std::cout << "Matrix A(1,2): " << testMat.getMatrixElement(1,2) << std::endl;

    std::cout << "Matrix A(2,0): " << testMat.getMatrixElement(2,0) << std::endl;
    std::cout << "Matrix A(2,1): " << testMat.getMatrixElement(2,1) << std::endl;
    std::cout << "Matrix A(2,2): " << testMat.getMatrixElement(2,2) << std::endl;

    std::cout << "Setting stuff in matrix A to create identity matrix: " << std::endl;

    testMat.setMatrixElement(0,0,1);
    testMat.setMatrixElement(0,1,0);
    testMat.setMatrixElement(0,2,0);

    testMat.setMatrixElement(1,0,0);
    testMat.setMatrixElement(1,1,1);
    testMat.setMatrixElement(1,2,0);

    testMat.setMatrixElement(2,0,0);
    testMat.setMatrixElement(2,1,0);
    testMat.setMatrixElement(2,2,1);

    std::cout << "Matrix A: " << std::endl;
    testMat.print(std::cout);







    std::cout <<  std::endl << "Testing A*b = c:" << std::endl;

    std::cout << "Matrix A: " << std::endl;
    testMat2.print(std::cout);

    std::cout << "Vector b: ";
    vecC.print(std::cout);

    MCISvector multResult = testMat2.rightMultiplyVector(vecC);

    std::cout << "Explicit function call   : ";
    multResult.print(std::cout);

    multResult = testMat2 * vecC;
    std::cout << "Operator*(matrix, vector): ";
    multResult.print(std::cout);





    std::cout <<  std::endl << "Testing matrix transpose" << std::endl;

    std::cout << "Matrix A: " << std::endl;
    testMat2.print(std::cout);

    testMat2.transpose();

    std::cout << "Transpose of A: " << std::endl;
    testMat2.print(std::cout);



    std::cout <<  std::endl << "Testing DCMs. Check the following results against this Matlab expression:" << std::endl;
    std::cout << "(angle2dcm(psi, theta, phi))' %Insert angles here, note the reverse order" << std::endl << std::endl;
    MCISvector euler{};
    MCISmatrix DCM{};

    euler.assign(0.5, 0.3, 2);
    DCM.euler2DCM_ZYX_inv(euler);

    std::cout << "Euler angles [phi, theta, psi]: ";
    euler.print(std::cout);
    DCM.print(std::cout);

    euler.assign(0.175, 0.087, 0.63);
    DCM.euler2DCM_ZYX_inv(euler);

    std::cout << "Euler angles [phi, theta, psi]: ";
    euler.print(std::cout);
    DCM.print(std::cout);

    euler.assign(0.087, 0.005, 0.02);
    DCM.euler2DCM_ZYX_inv(euler);

    std::cout << "Euler angles [phi, theta, psi]: ";
    euler.print(std::cout);
    DCM.print(std::cout);


    std::cout <<  std::endl << "Testing pqr2Euler." << std::endl << std::endl;
    MCISmatrix transform{};

    euler.assign(0.5, 0.3, 2);
    transform.pqr2eulerRates(euler);

    std::cout << "Euler angles [phi, theta, psi]: ";
    euler.print(std::cout);
    transform.print(std::cout);

    euler.assign(0.175, 0.087, 0.63);
    transform.pqr2eulerRates(euler);

    std::cout << "Euler angles [phi, theta, psi]: ";
    euler.print(std::cout);
    transform.print(std::cout);

    euler.assign(0.087, 0.005, 0.02);
    transform.pqr2eulerRates(euler);

    std::cout << "Euler angles [phi, theta, psi]: ";
    euler.print(std::cout);
    transform.print(std::cout);



    sleep(5);
}
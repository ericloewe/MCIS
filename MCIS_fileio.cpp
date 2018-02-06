

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <exception>
#include "discreteMath.h"
#include "MCIS_fileio.h"



/*
 *  readMCISinputs
 * 
 * Reads a set of six doubles to serve as inputs for MCIS
 * X,Y,Z acceleration, roll, pitch, yaw rates
 * 
 * Returns true on success and false on failure
 */
bool readMCISinputs(std::istream& infile, MCISvector& accIn, MCISvector& angIn)
{
    std::string stringbuffer;

    int i;
    double buffer[] = {0,0,0,0,0,0};

    /*
     *  There are 6 elements, yet we stop at 5, not 6. This is because
     * getline(), when given a specific delimiter to work with, will *ignore*
     * \n. This breaks things.
     * 
     * So, we need to read the last element of the row separately.
     * 
     * Yes, this is a pain in the ass. Yes, it's probably easier with the 
     * C standard library. *sigh*
     */
    for (i = 0; i < 5; i++)
    {
        if (std::getline(infile, stringbuffer, ','))
        {
            buffer[i] = stod(stringbuffer, nullptr);
        }
        else
        {
            break;
        }
    }

    if (i != 5) //We failed if we read less than 5 elements by now
    {           //We compare against 5 because i is incremented after the loop runs
        return false;
    }

    //Now we can deal with the last element of the row...
    if (std::getline(infile, stringbuffer)) //No delimiter == \n
    {
        buffer[5] = stod(stringbuffer, nullptr);
    }
    else
    {
        return false;
    }

    accIn.assign(buffer[0], buffer[1], buffer[2]);
    angIn.assign(buffer[3], buffer[4], buffer[5]);
    return true;
}

/*
 *  writeMCISoutputs
 * 
 * Writes a set of six doubles from the MCIS output
 * X,Y,Z position, roll, pitch, yaw
 */
void writeMCISoutputs(std::ostream& outfile, 
                    const MCISvector& accIn, 
                    const MCISvector& angIn)
{
    outfile << accIn.getVal(0) << ',' << accIn.getVal(1) << ',' << accIn.getVal(2) << ',';
    outfile << angIn.getVal(0) << ',' << angIn.getVal(1) << ',' << angIn.getVal(2);
}

/*
 *  writeMCISfullOutputs
 * 
 * Writes a set of six doubles from the MCIS output
 * X,Y,Z position, roll, pitch, yaw
 */
void writeMCISfullOutputs(std::ostream& outfile, 
                        const MCISvector& accIn, 
                        const MCISvector& angIn,
                        const MCISvector& noTCin)
{
    writeMCISoutputs(outfile, accIn, angIn);
    outfile << ',' << noTCin.getVal(0) << ',' << noTCin.getVal(1) << ',' << noTCin.getVal(2);
}
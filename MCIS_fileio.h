
#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <exception>
#include "discreteMath.h"


/*
 *  readMCISinputs
 * 
 * Reads a set of six doubles to serve as inputs for MCIS
 * X,Y,Z acceleration, roll, pitch, yaw rates
 */
bool readMCISinputs(std::istream& infile, MCISvector& accIn, MCISvector& angIn);

/*
 *  writeMCISoutputs
 * 
 * Writes a set of six doubles from the MCIS output
 * X,Y,Z position, roll, pitch, yaw
 */
void writeMCISoutputs(std::ostream& outfile, 
                    const MCISvector& accIn, 
                    const MCISvector& angIn);

/*
 *  writeBaseMCISoutputs
 * 
 * Writes a set of six doubles from the MCIS output
 * X,Y,Z position, roll, pitch, yaw
 * 
 * Actual heavy lifting goes here
 */
void writeBaseMCISoutputs(std::ostream& outfile, 
                    const MCISvector& accIn, 
                    const MCISvector& angIn);

/*
 *  writeMCISfullOutputs
 * 
 * Writes a set of six doubles from the MCIS output
 * X,Y,Z position, roll, pitch, yaw
 */
void writeMCISfullOutputs(std::ostream& outfile, 
                        const MCISvector& accIn, 
                        const MCISvector& angIn,
                        const MCISvector& noTCin);
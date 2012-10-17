/*    Copyright (c) 2010-2012, Delft University of Technology
 *    All rights reserved.
 *
 *    Redistribution and use in source and binary forms, with or without modification, are
 *    permitted provided that the following conditions are met:
 *      - Redistributions of source code must retain the above copyright notice, this list of
 *        conditions and the following disclaimer.
 *      - Redistributions in binary form must reproduce the above copyright notice, this list of
 *        conditions and the following disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *      - Neither the name of the Delft University of Technology nor the names of its contributors
 *        may be used to endorse or promote products derived from this software without specific
 *        prior written permission.
 *
 *    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
 *    OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *    MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *    COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *    EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 *    GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 *    AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *    NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 *    OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *    Changelog
 *      110530    F.M. Engelen      First creation of code.
 *      120326    D. Dirkx          Modified code to be consistent with latest Tudat/TudatCore;
 *                                  moved relevant functionality of (text)FileReader to this class.
 *
 *    References
 *      Blake, W.B. Missile Datcom User's Manual - 1997 Fortran 90 Version,
 *          AFRL-VA-WP-TR-1998-3009 Air Force Research Laboratory, 1998.
 *
 */

#ifndef TUDAT_MISSILE_DATCOM_READER_H
#define TUDAT_MISSILE_DATCOM_READER_H

#include <vector>
#include <fstream>
#include <string>
#include <map>

namespace tudat
{
namespace input_output
{

//! Class to read a for004.dat file generated by missile Datcom.
/*!
 * Class to read a for004.dat file generated by missile Datcom.
 * Based on the input cards
 * FLC,1,145    (Flight Condition Data)
 * SB1,1,220    (Static Coefficient and Derivative Data)
 * DB1,1,400    (Dynamic Derivative Data)
 */
class MissileDatcomReader
{
public:

    //! Class constructor, reads data file.
    /*!
     * Class constructor, reads data file to vector of doubles containing all data.
     * \param fileNameAndPath Path and name of file containing missile datcom data
     */
    MissileDatcomReader( const std::string& fileNameAndPath );

    //! Gets the split and parsed data from the 004 file
    /*!
     *  Gets the split and parsed data from the 004 file.
     *  \return Vector of doubles, which have been sequentially read from 004 file.
     */
    std::vector< double > getMissileDatcomData( ){ return missileDatcomData_; }

protected:

private:

    //! Function to read the for004.dat file and return one long std::vector
    /*!
     * Function to read the for004.dat file and return one long std::vector
     * with first the FLC data, then the SB1 data, next the DB1 data, and then the
     * same data again for the next machnumber. (see missile datcom user manual p88 and further).
     * \param fileNameAndPath Path and name of the for004.dat file.
     */
    void readFor004( const std::string& fileNameAndPath );

    //! Open data file.
    /*!
     * Opens the for004.dat data file.
     * \param fileNameAndPath Path and name of the for004.dat file.
     */
    void openFile( const std::string& fileNameAndPath );

    //! Function to split a single std::string into a std::vector of std::strings
    /*!
     * Function to split a single std::string into a std::vector of small std::strings.
     * Entries are separated by a separator (which is also a std::string)
     * WARNING: If multiple seperators are placed next to each other, the entry is skipped.
     * \param dataString the long std::string which needs to be splitted.
     * \param separator the separator symbol
     * \param dataVector the std::vector with all the substd::strings.
     */
    void split( const std::string& dataString, char separator,
                std::vector< std::string >& dataVector );

    //! Read and store data.
    /*!
     * Reads and stores data from input file.
     * \param skipKeyword Keyword to skip in input file.
     */
    void readAndStoreData( const std::string& skipKeyword );

    //! Map to store the read for004.dat file.
    /*!
     * Map to store the read for004.dat file.
     */
    std::map< unsigned int, std::string > missileDatcomDataContainer_;

    //! Iterator for std::map container of data from file.
    /*!
     * Iterator for std::map container of std::string data from data file.
     */
    std::map< unsigned int, std::string >::iterator iteratorContainerOfData_;

    //! String to temporarilly store the a splitted std::string.
    /*!
     * String to temporarilly store the a splitted std::string.
     */
    std::vector< std::string > dataVector_;

    //! Data std::vector with the rough split and parsed missileDatcom data.
    /*!
     *  Data std::vector with the rough split and parsed missileDatcom data.
     */
    std::vector< double > missileDatcomData_;

    //! Data file stream.
    /*!
     * Data file stream.
     */
    std::ifstream dataFile_;

    //! Map container of data from file.
    /*!
     * Map container of string data from data file, obtained by reading each
     * line of data from file using the getline( ) function. The key is the
     * line number from the file and the value is line data.
     */
    std::map< unsigned int, std::string > containerOfDataFromFile_;

    //! Function to Convert a string to a double.
    /*!
     * Creates an istringstream object and converted the string to a double. Throws an exception
     * if incorrect string is used.
     * \param inputString string with the data to be converted
     * \return the converted string
     */
    double stringToDouble( std::string const& inputString );
};

} // namespace input_output
} // namespace tudat

#endif // TUDAT_MISSILE_DATCOM_READER_H
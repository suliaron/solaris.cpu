#include <iomanip>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <list>
#include <string>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

#include "BinaryFileAdapter.h"
#include "Body.h"
#include "BodyGroup.h"
#include "Output.h"
#include "Phase.h"
#include "Tools.h"
#include "TwoBodyAffair.h"

using namespace std;

int BinaryFileAdapter::_propertyId = 0;
int BinaryFileAdapter::_compositionId = 0;

BinaryFileAdapter::BinaryFileAdapter(Output *output) 
{
	this->output = output;
}

bool BinaryFileAdapter::FileExists(const string& name) {
    struct stat buffer;
    string path = output->GetPath(name);
    return (stat (path.c_str(), &buffer) == 0); 
}

/**
 * Write the message to the log file and if printToScreen = true than also to the screen. On error
 * the function exits to the system.
 *
 * @param msg message to write into the log file
 * @param printToScreen if true the message will be also printed to the screen
 */
void BinaryFileAdapter::Log(string msg, bool printToScreen)
{
	char dateTime[20];
	time_t now = time(0);
	strftime(dateTime, 20, "%Y-%m-%d %H:%M:%S", localtime(&now));

	string path = output->GetPath(output->log);
	ofstream logger(path.c_str(), ios::out | ios::app);
	if (logger) {
		logger << dateTime << " " << msg << endl;
		if (logger.bad()) {
			perror(("error while reading file " + path).c_str());
		}
		logger.close();
	}
	else {
		cerr << "The file '" << path << "' could not opened!\r\nExiting to system!" << endl;
		exit(1);
	}

	if (printToScreen) {
		cerr << dateTime << " " << msg << endl;
	}
}

/**
 * Writes the command line arguments to the log file.
 *
 * @param argc contains the number of arguments passed to the program
 * @param argv is a one-dimensional array of strings, each string is one of the arguments that was passed to the program
 * @param binaryFileAdapter file adapter to write the data to the log file
 */
void BinaryFileAdapter::LogStartParameters(int argc, char* argv[])
{
	string commandLine = "CommandLine:";
	for (int i=0; i<argc; i++) {
		commandLine += ' ';
		commandLine += argv[i];
	}
	Log(commandLine, false);
}

/**
 * Writes the elapsed wall time between the function call and the parameter t0
 *
 * @param msg the message to write to the log file
 * @param t0 the reference wall time from wgich the elapsed time is calculated
 */
void BinaryFileAdapter::LogTimeSpan(string msg, time_t t0)
{
	int diff = (int)(time(0) - t0);

	string diffTime;
	Tools::TimeDifference(diff, diffTime);
	msg += diffTime;
	Log(msg, true);
}

/// <summary>
/// Save the phases of the bodies defined in the list parameter into the file defined
/// by the path parameter. The format of the output file is binary.
/// </summary>
/// <param name="path">The path of the output file</param>
/// <param name="list">The list of the bodies whose phases will be saved</param>
void BinaryFileAdapter::SavePhases(double time, int n, double *y, int *id, output_type_t type, bool& phasenew)
{
	switch (type) 
	{
		case OUTPUT_TYPE_BINARY:
		{
			string path = output->GetPath(output->phases);
			ofstream writer;
			if (phasenew) {
				writer.open(path.c_str(), ios::out | ios::binary);
				phasenew = false;
			}
			else {
				writer.open(path.c_str(), ios::out | ios::app | ios::binary);
			}			
			if (writer) {
				writer.write(reinterpret_cast<char*>(&time), sizeof(time));
				writer.write(reinterpret_cast<char*>(&n),    sizeof(n));
				for (register int i=0; i<n; i++) {
					SavePhase(writer, &(y[6*i]), &(id[i]), type);
				}
				writer.flush();
				writer.close();
			}
			else {
				Log("The file '" + path + "' could not opened!", true);
				exit(1);
			}
			break;
		}
	case OUTPUT_TYPE_TEXT:
		{
			string path = output->GetPath(output->GetFilenameWithoutExt(output->phases) + ".txt");
			ofstream writer;
			if (phasenew) {
				writer.open(path.c_str(), ios::out);
				phasenew = false;
			}
			else {
				writer.open(path.c_str(), ios::out | ios::app);
			}
			if (writer) {
				writer << setw(15) << setprecision(10) << time;
				writer << setw(8) << n;
				for (register int i=0; i<n; i++) {
					SavePhase(writer, &(y[6*i]), &(id[i]), type);
				}
				writer << endl;
				writer.flush();
				writer.close();
			}
			else {
				Log("The file '" + path + "' could not opened!", true);
				exit(1);
			}
			break;
		}
	default:
		Log("Unknown OutputType.", true);
		exit(1);
		break;
	}

    //cout << "At " << setw(10) << time * Constants::DayToYear << " [yr] phases were saved" << endl;
}

void BinaryFileAdapter::SavePhase(ofstream& writer, double *y, int *id, output_type_t type)
{
	switch(type)
	{
	case OUTPUT_TYPE_BINARY:
		writer.write(reinterpret_cast<char*>(id), sizeof(*id));
		writer.write(reinterpret_cast<char*>(y),  6*sizeof(*y));
		break;
	case OUTPUT_TYPE_TEXT:
		writer << setw(8) << *id;
		for (int i = 0; i < 6; i++) {
			writer << setw(15) << setprecision(6) << y[i];
		}
		break;
	}
	if (writer.bad()) {
		_errMsg = "An error occurred during writing the phase!";
		Log(_errMsg, true);
		perror(_errMsg.c_str());
		exit(1);
	}
}

/**
 * Saves the energy, the angular momentum vector and its length, the position vector of the baricenter and its length
 * and the velocity of the barycenter and its length.
 */
void BinaryFileAdapter::SaveIntegrals(double time, int n, double *integrals, output_type_t type, bool& integralnew)
{

	switch(type)
	{
		case OUTPUT_TYPE_BINARY:
		{
			static bool firstCall = true;

			string path = output->GetPath(output->integrals);
			ofstream writer;
			if (integralnew) {
				writer.open(path.c_str(), ios::out | ios::binary);
				integralnew = false;
			}
			else {
				writer.open(path.c_str(), ios::out | ios::app | ios::binary);
			}
			if (writer) {
				if (firstCall)
				{
					char header[] = "time [day], mass [Solar], bary center position r: x [AU], y [AU], z [AU], bary center velocity v: x [AU/day], y [AU/day], z [AU/day], length of r [AU], length of v [AU/day], angular momentum vector c: x, y, z, length of c, kinetic energy, potential energy, total energy";
					int len = (int)strlen(header);
					writer.write(reinterpret_cast<char*>(&len), sizeof(int));
					writer.write(header, len);
					firstCall = false;
				}
				int nElement = n + 1;
				writer.write(reinterpret_cast<char*>(&nElement), sizeof(nElement));
				writer.write(reinterpret_cast<char*>(&time), sizeof(time));
				writer.write(reinterpret_cast<char*>(integrals), n * sizeof(*integrals));
				if (writer.bad()) {
					_errMsg = "An error occurred during writing the integrals!";
					Log(_errMsg, true);
					perror(_errMsg.c_str());
					exit(1);
				}
			}
			else {
				Log("The file '" + path + "' could not opened!", true);
				exit(1);
			}
			writer.close();
			break;
		}

		case OUTPUT_TYPE_TEXT:
		{
			string path = output->GetPath(output->GetFilenameWithoutExt(output->integrals) + ".txt");
			ofstream writer;
			if (integralnew) {
				writer.open(path.c_str(), ios::out);
				integralnew = false;
			}
			else {
				writer.open(path.c_str(), ios::out | ios::app);
			}
			if (writer) {
				writer << setw(15) << setprecision(6) << time;
				for (int i = 0; i < 16; i++) {
					writer << setw(15) << setprecision(6) << integrals[i];
				}
				writer << endl;
				if (writer.bad()) {
					_errMsg = "An error occurred during writing the integrals!";
					Log(_errMsg, true);
					perror(_errMsg.c_str());
					exit(1);
				}
			}
			else {
				Log("The file '" + path + "' could not opened!", true);
				exit(1);
			}
			writer.close();
			break;
		}
		default:
			Log("Unknown OutputType.", true);
			exit(1);
			break;
	}
}

/// <summary>
/// Writes the list of TwoBodyAffairs data to the disk.
/// </summary>
/// <param name="path">The path of the output file</param>
/// <param name="list">The data of the TwoBodyAffairs</param>
void BinaryFileAdapter::SaveTwoBodyAffairs(list<TwoBodyAffair>& list, output_type_t type, bool& twobodyaffnew)
{
	switch(type)
	{
		case OUTPUT_TYPE_BINARY:
		{
			string path = output->GetPath(output->twoBodyAffair);
			ofstream writer;
			if (twobodyaffnew) {
				writer.open(path.c_str(), ios::out | ios::binary);
				twobodyaffnew = false;
			}
			else {
				writer.open(path.c_str(), ios::out | ios::app | ios::binary);
			}
			if (writer) {
				for (std::list<TwoBodyAffair>::iterator it = list.begin(); it != list.end(); it++) {
					SaveTwoBodyAffair(writer, *it, type);
				}
				writer.close();
			}
			else {
				_errMsg = "The file '" + path + "' could not opened!";
				Log(_errMsg, true);
				perror(_errMsg.c_str());
				exit(1);
			}
			break;
		}

		case OUTPUT_TYPE_TEXT:
		{
			string path = output->GetPath(output->GetFilenameWithoutExt(output->twoBodyAffair) + ".txt");
			ofstream writer;
			if (twobodyaffnew) {
				writer.open(path.c_str(), ios::out);
				twobodyaffnew = false;
			}
			else {
				writer.open(path.c_str(), ios::out | ios::app);
			}
			if (writer) {
				for (std::list<TwoBodyAffair>::iterator it = list.begin(); it != list.end(); it++) {
					SaveTwoBodyAffair(writer, *it, type);
				}
				writer.close();
			}
			else {
				_errMsg = "The file '" + path + "' could not opened!";
				Log(_errMsg, true);
				perror(_errMsg.c_str());
				exit(1);
			}
			break;
		}
		default:
			Log("Unknown OutputType.", true);
			exit(1);
			break;
	}
	
	
}

void BinaryFileAdapter::SaveTwoBodyAffair(ofstream& writer, TwoBodyAffair& affair, output_type_t type)
{
	switch(type)
	{
		case OUTPUT_TYPE_BINARY:
		{
			writer.write(reinterpret_cast<char*>(&(affair.id)), sizeof(int));
			writer.write(reinterpret_cast<char*>(&(affair.type)), sizeof(int));

			writer.write(reinterpret_cast<char*>(&(affair.body1Id)), sizeof(int));
			writer.write(reinterpret_cast<char*>(&(affair.body2Id)), sizeof(int));
			writer.write(reinterpret_cast<char*>(&(affair.body1Phase)), 6*sizeof(double));
			writer.write(reinterpret_cast<char*>(&(affair.body2Phase)), 6*sizeof(double));
			writer.write(reinterpret_cast<char*>(&(affair.time)), sizeof(double));

			break;
		}

		case OUTPUT_TYPE_TEXT:
		{
			writer << setw(5) << affair.id;
			writer << setw(5) << affair.type;
			writer << setw(5) << affair.body1Id;
			writer << setw(5) << affair.body2Id;
			for (int i = 0; i < 6; i++) {
				writer << setw(15) << setprecision(6) << affair.body1Phase[i];
			}
			for (int i = 0; i < 6; i++) {
				writer << setw(15) << setprecision(6) << affair.body2Phase[i];
			}
			writer << setw(15) << setprecision(6) << affair.time;
			break;
		}
	}
	
	if (writer.bad()) {
		_errMsg = "An error occurred during writing the two body affair!";
		Log(_errMsg, true);
		perror(_errMsg.c_str());
		exit(1);
	}
}

void BinaryFileAdapter::SaveBodyProperties(double time, list<Body* >& bodyList, output_type_t type, bool& constpropnew, bool& varpropnew)
{
	switch(type)
	{
		case OUTPUT_TYPE_BINARY:
		{
			string constPropPath = output->GetPath(output->constantProperties);
			ofstream constPropWriter;
			if (constpropnew) {
				constPropWriter.open(constPropPath.c_str(), ios::out | ios::binary);
				constpropnew = false;
			}
			else {
				constPropWriter.open(constPropPath.c_str(), ios::out | ios::app | ios::binary);
			}
			if (!constPropWriter) {
				_errMsg = "The file '" + constPropPath + "' could not opened!";
				Log(_errMsg, true);
				perror(_errMsg.c_str());
				exit(1);
			}

			string varPropPath = output->GetPath(output->variableProperties);
			ofstream varPropWriter;
			if (varpropnew) {
				varPropWriter.open(varPropPath.c_str(), ios::out | ios::binary);
				varpropnew = false;
			}
			else {
				varPropWriter.open(varPropPath.c_str(), ios::out | ios::app | ios::binary);
			}

			if (!varPropWriter) {
				_errMsg = "The file '" + varPropPath + "' could not opened!";
				Log(_errMsg, true);
				perror(_errMsg.c_str());
				constPropWriter.close();
				exit(1);
			}

			for (list<Body* >::iterator it = bodyList.begin(); it != bodyList.end(); it++) {
				SaveConstantProperty(constPropWriter, *it, type);
				SaveVariableProperty(varPropWriter, *it, time, type, varpropnew);
			}
			constPropWriter.close();
			varPropWriter.close();
			break;
		}
		case OUTPUT_TYPE_TEXT:
		{
			string constPropPath = output->GetPath(output->GetFilenameWithoutExt(output->constantProperties) + ".txt");
			ofstream constPropWriter;
			if (constpropnew) {
				constPropWriter.open(constPropPath.c_str(), ios::out);
				constpropnew = false;
			}
			else {
				constPropWriter.open(constPropPath.c_str(), ios::out | ios::app);
			}
			if (!constPropWriter) {
				_errMsg = "The file '" + constPropPath + "' could not opened!";
				Log(_errMsg, true);
				perror(_errMsg.c_str());
				exit(1);
			}

			string varPropPath = output->GetPath(output->GetFilenameWithoutExt(output->variableProperties) + ".txt");
			ofstream varPropWriter;
			if (varpropnew) {
				varPropWriter.open(varPropPath.c_str(), ios::out);
				varpropnew = false;
			}
			else {
				varPropWriter.open(varPropPath.c_str(), ios::out | ios::app | ios::binary);
			}
			if (!varPropWriter) {
				_errMsg = "The file '" + varPropPath + "' could not opened!";
				Log(_errMsg, true);
				perror(_errMsg.c_str());
				constPropWriter.close();
				exit(1);
			}

			for (list<Body* >::iterator it = bodyList.begin(); it != bodyList.end(); it++) {
				SaveConstantProperty(constPropWriter, *it, type);
				SaveVariableProperty(varPropWriter, *it, time, type, varpropnew);
			}
			constPropWriter.close();
			varPropWriter.close();
			break;
		}
	}

	
}

void BinaryFileAdapter::SaveConstantProperty(Body* body, output_type_t type, bool& constpropnew)
{
	switch(type)
	{
		case OUTPUT_TYPE_BINARY:
		{
			string constPropPath = output->GetPath(output->constantProperties);
			ofstream constPropWriter;
			if (constpropnew) {
				constPropWriter.open(constPropPath.c_str(), ios::out | ios::binary);
				constpropnew = false;
			}
			else {
				constPropWriter.open(constPropPath.c_str(), ios::out | ios::app | ios::binary);
			}
			if (!constPropWriter) {
				_errMsg = "The file '" + constPropPath + "' could not opened!";
				Log(_errMsg, true);
				perror(_errMsg.c_str());
				exit(1);
			}
			SaveConstantProperty(constPropWriter, body, type);
			constPropWriter.close();
			break;
		}
		case OUTPUT_TYPE_TEXT:
		{
			string constPropPath = output->GetPath(output->GetFilenameWithoutExt(output->constantProperties) + ".txt");
			ofstream constPropWriter;
			if (constpropnew) {
				constPropWriter.open(constPropPath.c_str(), ios::out);
				constpropnew = false;
			}
			else {
				constPropWriter.open(constPropPath.c_str(), ios::out | ios::app);
			}
			if (!constPropWriter) {
				_errMsg = "The file '" + constPropPath + "' could not opened!";
				Log(_errMsg, true);
				perror(_errMsg.c_str());
				exit(1);
			}
			SaveConstantProperty(constPropWriter, body, type);
			constPropWriter.close();
			break;
		}
	}

	
}

void BinaryFileAdapter::SaveConstantProperty(ofstream& writer, Body* body, output_type_t type)
{
	switch(type)
	{
		case OUTPUT_TYPE_BINARY:
		{
			int i = body->GetId();
			writer.write(reinterpret_cast<char *>(&i), sizeof(i));

			string s = body->guid;
			char *guid = new char[16];
			Tools::GuidToCharArray(s, guid);

			// NOTE: this must be done in such a way that when the C# initializes a new guid form an array of 16 bytes, the correct
			// guid will be created. The constructor : Guid(int, short, short, byte, byte, byte, byte, byte, byte, byte, byte)
			// This will be the int part
			for (int j=3; j>=0; j--) writer.write(&guid[j], 1);
			// This will be the 1. short part
			for (int j=5; j>=4; j--) writer.write(&guid[j], 1);
			// This will be the 2. short part
			for (int j=7; j>=6; j--) writer.write(&guid[j], 1);
			// This will be the last 8 bytes
			writer.write(&guid[8], 8);
			//writer.write(guid, 16);

			// NOTE: The length of the strings cannot be more than 127!
			// See: http://stackoverflow.com/questions/1550560/encoding-an-integer-in-7-bit-format-of-c-sharp-binaryreader-readstring
			// because I will read this file with C#, and BinaryReader.ReadString method use a 7-bit encoding for the length of the string.
			const char *p = body->name.c_str();
			char len = (char)strlen(p);
			writer.write(&len, sizeof(char));
			writer.write(p, len);

			p = body->designation.c_str();
			len = (char)strlen(p);
			writer.write(&len, sizeof(char));
			writer.write(p, len);

			p = body->provisionalDesignation.c_str();
			len = (char)strlen(p);
			writer.write(&len, sizeof(char));
			writer.write(p, len);

			p = body->reference.c_str();
			len = (char)strlen(p);
			writer.write(&len, sizeof(char));
			writer.write(p, len);

			p = body->opposition.c_str();
			len = (char)strlen(p);
			writer.write(&len, sizeof(char));
			writer.write(p, len);

			i = body->ln;
			writer.write(reinterpret_cast<char *>(&i), sizeof(i));

			i = body->type;
			writer.write(reinterpret_cast<char *>(&i), sizeof(i));

			i = body->mPCOrbitType;
			writer.write(reinterpret_cast<char *>(&i), sizeof(i));

			i = body->migrationType;
			writer.write(reinterpret_cast<char *>(&i), sizeof(i));

			if (body->characteristics != 0) {
				double d = body->characteristics->absVisMag;
				writer.write(reinterpret_cast<char *>(&d), sizeof(d));

				d = body->characteristics->stokes;
				writer.write(reinterpret_cast<char *>(&d), sizeof(d));
			}
			else {
				double d = 0.0;
				writer.write(reinterpret_cast<char *>(&d), sizeof(d));
				writer.write(reinterpret_cast<char *>(&d), sizeof(d));
			}
			break;
		}

		case OUTPUT_TYPE_TEXT:
		{
			writer << setw(5) << body->GetId();
			writer << setw(10) << body->name;
			writer << setw(10) << body->designation;
			writer << setw(10) << body->provisionalDesignation;
			writer << setw(10) << body->reference;
			writer << setw(10) << body->opposition;
			writer << setw(3) << body->ln;
			writer << setw(3) << body->mPCOrbitType;
			writer << setw(3) << body->migrationType;
			if (body->characteristics != 0) {
				writer << setw(10) << setprecision(6) << body->characteristics->absVisMag;
				writer << setw(10) << setprecision(6) << body->characteristics->stokes;
				writer << endl;
			}
			else {
				writer << setw(10) << 0.0;
				writer << setw(10) << 0.0;
				writer << endl;
			}
			break;
		}
	}

	if (writer.bad()) {
		_errMsg = "An error occurred during writing the constant property!";
		Log(_errMsg, true);
		perror(_errMsg.c_str());
		exit(1);
	}
}

void BinaryFileAdapter::SaveVariableProperty(Body* body, double time, output_type_t type, bool& varpropnew)
{
	switch(type)
	{
		case OUTPUT_TYPE_BINARY:
		{
			string varPropPath = output->GetPath(output->variableProperties);
			ofstream varPropWriter;
			if (varpropnew) {
				varPropWriter.open(varPropPath.c_str(), ios::out | ios::binary);
				varpropnew = false;
			}
			else {
				varPropWriter.open(varPropPath.c_str(), ios::out | ios::app | ios::binary);
			}
			if (!varPropWriter) {
				_errMsg = "The file '" + varPropPath + "' could not opened!";
				Log(_errMsg, true);
				perror(_errMsg.c_str());
				exit(1);
			}
			SaveVariableProperty(varPropWriter, body, time, type, varpropnew);
			varPropWriter.close();
			break;
		}
		case OUTPUT_TYPE_TEXT:
		{
			string varPropPath = output->GetPath(output->GetFilenameWithoutExt(output->variableProperties) + ".txt");
			ofstream varPropWriter;
			if (varpropnew) {
				varPropWriter.open(varPropPath.c_str(), ios::out);
				varpropnew = false;
			}
			else {
				varPropWriter.open(varPropPath.c_str(), ios::out | ios::app);
			}
			if (!varPropWriter) {
				_errMsg = "The file '" + varPropPath + "' could not opened!";
				Log(_errMsg, true);
				perror(_errMsg.c_str());
				exit(1);
			}
			SaveVariableProperty(varPropWriter, body, time, type, varpropnew);
			varPropWriter.close();
			break;
		}
	}
	
	
}

void BinaryFileAdapter::SaveVariableProperty(ofstream& writer, Body* body, double time, output_type_t type, bool& comppropnew)
{
	switch(type)
	{
		case OUTPUT_TYPE_BINARY:
		{
			int id = _propertyId++;
			writer.write(reinterpret_cast<char *>(&id), sizeof(id));

			int bodyId = body->GetId();
			writer.write(reinterpret_cast<char *>(&bodyId), sizeof(bodyId));

			double d = time;
			writer.write(reinterpret_cast<char *>(&d), sizeof(d));

			if (body->characteristics != 0) {
				d = body->characteristics->mass;
				writer.write(reinterpret_cast<char *>(&d), sizeof(d));

				d = body->characteristics->radius;
				writer.write(reinterpret_cast<char *>(&d), sizeof(d));

				d = body->characteristics->density;
				writer.write(reinterpret_cast<char *>(&d), sizeof(d));
				if (body->characteristics->componentList.size() > 0) {
					string compPropPath = output->GetPath(output->compositionProperties);
					ofstream compPropWriter;
					if (comppropnew) {
						compPropWriter.open(compPropPath.c_str(), ios::out | ios::binary);
						comppropnew = false;
					}
					else {
						compPropWriter.open(compPropPath.c_str(), ios::out | ios::app | ios::binary);
					}
					if (!compPropWriter) {
						_errMsg = "The file '" + compPropPath + "' could not opened!";
						Log(_errMsg, true);
						perror(_errMsg.c_str());
						exit(1);
					}
					SaveCompositionProperty(compPropWriter, body, id, type);
					compPropWriter.close();
				}
			}
			else {
				double d = 0.0;
				writer.write(reinterpret_cast<char *>(&d), sizeof(d));
				writer.write(reinterpret_cast<char *>(&d), sizeof(d));
				writer.write(reinterpret_cast<char *>(&d), sizeof(d));
			}
			if (writer.bad()) {
				_errMsg = "An error occurred during writing the variable property!";
				Log(_errMsg, true);
				perror(_errMsg.c_str());
				exit(1);
			}
			break;
		}
		case OUTPUT_TYPE_TEXT:
		{
			int id = _propertyId++;
			writer << setw(5) << id;
			writer << setw(5) << body->GetId();
			writer << setw(15) << setprecision(6) << time;
			if (body->characteristics != 0) {
				writer << setw(15) << setprecision(6) << body->characteristics->mass;
				writer << setw(15) << setprecision(6) << body->characteristics->radius;
				writer << setw(15) << setprecision(6) << body->characteristics->density;
				writer << endl;
				if (body->characteristics->componentList.size() > 0) {
					string compPropPath = output->GetPath(output->GetFilenameWithoutExt(output->compositionProperties) + ".txt");
					ofstream compPropWriter;
					if (comppropnew) {
						compPropWriter.open(compPropPath.c_str(), ios::out);
						comppropnew = false;
					}
					else {
						compPropWriter.open(compPropPath.c_str(), ios::out | ios::app);
					}
					if (!compPropWriter) {
						_errMsg = "The file '" + compPropPath + "' could not opened!";
						Log(_errMsg, true);
						perror(_errMsg.c_str());
						exit(1);
					}
					SaveCompositionProperty(compPropWriter, body, id, type);
					compPropWriter.close();
				}
			}
			else {
				writer << setw(15) << 0.0;
				writer << setw(15) << 0.0;
				writer << setw(15) << 0.0;
				writer << endl;
			}
			if (writer.bad()) {
				_errMsg = "An error occurred during writing the variable property!";
				Log(_errMsg, true);
				perror(_errMsg.c_str());
				exit(1);
			}
			break;
		}
	}
	
}

void BinaryFileAdapter::SaveCompositionProperty(ofstream& writer, Body* body, int propertyId, output_type_t type)
{
	switch(type)
	{
		case OUTPUT_TYPE_BINARY:
		{
			for (list<Component>::iterator it = body->characteristics->componentList.begin(); it != body->characteristics->componentList.end(); it++) {

				int id = _compositionId++;
				writer.write(reinterpret_cast<char *>(&id), sizeof(id));

				id = propertyId;
				writer.write(reinterpret_cast<char *>(&id), sizeof(id));

				const char *p = it->name.c_str();
				char len = (char)strlen(p);
				writer.write(&len, sizeof(char));
				writer.write(p, len);

				double d = it->ratio;
				writer.write(reinterpret_cast<char *>(&d), sizeof(d));

				if (writer.bad()) {
					_errMsg = "An error occurred during writing the composition property!";
					Log(_errMsg, true);
					perror(_errMsg.c_str());
					exit(1);
				}
			}
			break;
		}
		case OUTPUT_TYPE_TEXT:
		{
			for (list<Component>::iterator it = body->characteristics->componentList.begin(); it != body->characteristics->componentList.end(); it++) {

				int id = _compositionId++;
				writer << setw(5) << id;
				writer << setw(5) << propertyId;
				
				writer << setw(10) << it->name;
				writer << setw(10) << setprecision(6) << it->ratio;
				writer << endl;
				
				if (writer.bad()) {
					_errMsg = "An error occurred during writing the composition property!";
					Log(_errMsg, true);
					perror(_errMsg.c_str());
					exit(1);
				}
			}
			break;
		}
	}
}

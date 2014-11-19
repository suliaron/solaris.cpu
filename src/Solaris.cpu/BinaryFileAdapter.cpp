#include <iomanip>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdint>
#include <cstdio>
#include <list>
#include <sstream>
#include <string>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

#include "BinaryFileAdapter.h"
#include "Body.h"
#include "BodyData.h"
#include "BodyGroup.h"
#include "Counter.h"
#include "Ephemeris.h"
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
void BinaryFileAdapter::SavePhases(double time, int n, double *y, int *id, output_type_t type, int removed)
{
	switch (type) 
	{
		case OUTPUT_TYPE_BINARY:
		{
			string path = output->GetPath(output->phases);
			ofstream writer;

			static bool firstcall = true;

			if (firstcall) {
				writer.open(path.c_str(), ios::out | ios::binary);
				firstcall = false;
			}
			else {
				writer.open(path.c_str(), ios::out | ios::app | ios::binary);
			}			
			if (writer) {
				writer.write(reinterpret_cast<char*>(&time), sizeof(time));
				writer.write(reinterpret_cast<char*>(&n),    sizeof(n));
				for (register int i=0; i<n; i++) {
					SavePhase(writer, &(y[6*i]), &(id[i]), type, removed);
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
			static bool firstcall = true;

			if (firstcall) {
				writer.open(path.c_str(), ios::out);
				firstcall = false;
			}
			else {
				writer.open(path.c_str(), ios::out | ios::app);
			}
			if (writer) {
				writer << setw(15) << setprecision(10) << time;
				writer << setw(8) << n;
				for (register int i=0; i < n; i++) {
					SavePhase(writer, &(y[6*i]), &(id[i]), type, 0);
				}
				if (removed > 0) {
					for (register int i=0; i < removed; i++) {
						SavePhase(writer, &(y[6*i]), &(id[i]), type, removed);
					}				
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

void BinaryFileAdapter::SavePhase(ofstream& writer, double *y, int *id, output_type_t type, int removed)
{
	switch(type)
	{
	case OUTPUT_TYPE_BINARY:
		writer.write(reinterpret_cast<char*>(id), sizeof(*id));
		writer.write(reinterpret_cast<char*>(y),  6*sizeof(*y));
		break;
	case OUTPUT_TYPE_TEXT:
		if (removed > 0) {
			writer << setw(8) << -(*id);
			for (int i = 0; i < 6; i++)
			{
				writer << setw(15) << 0;
			}
			break;
		}
		writer << setw(8) << *id;
		for (int i = 0; i < 6; i++)
		{
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
void BinaryFileAdapter::SaveIntegrals(double time, int n, double *integrals, output_type_t type)
{

	switch(type)
	{
		case OUTPUT_TYPE_BINARY:
		{
			static bool firstcall = true;

			string path = output->GetPath(output->integrals);
			ofstream writer;
			if (firstcall) {
				writer.open(path.c_str(), ios::out | ios::binary);
				firstcall = false;
			}
			else {
				writer.open(path.c_str(), ios::out | ios::app | ios::binary);
			}
			if (writer) {
				if (firstcall)
				{
					char header[] = "time [day], mass [Solar], bary center position r: x [AU], y [AU], z [AU], bary center velocity v: x [AU/day], y [AU/day], z [AU/day], length of r [AU], length of v [AU/day], angular momentum vector c: x, y, z, length of c, kinetic energy, potential energy, total energy";
					int len = (int)strlen(header);
					writer.write(reinterpret_cast<char*>(&len), sizeof(int));
					writer.write(header, len);
					firstcall = false;
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
			static bool firstcall = true;

			if (firstcall) {
				writer.open(path.c_str(), ios::out);
				firstcall = false;
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
void BinaryFileAdapter::SaveTwoBodyAffairs(list<TwoBodyAffair>& list, output_type_t type)
{
	switch(type)
	{
		case OUTPUT_TYPE_BINARY:
		{
			string path = output->GetPath(output->twoBodyAffair);
			ofstream writer;
			static bool firstcall = true;

			if (firstcall) {
				writer.open(path.c_str(), ios::out | ios::binary);
				firstcall = false;
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
			static bool firstcall = true;

			if (firstcall) {
				writer.open(path.c_str(), ios::out);
				firstcall = false;
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
			writer << setw(15) << setprecision(6) << affair.time;
			for (int i = 0; i < 6; i++) {
				writer << setw(15) << setprecision(6) << affair.body1Phase[i];
			}
			for (int i = 0; i < 6; i++) {
				writer << setw(15) << setprecision(6) << affair.body2Phase[i];
			}
			writer << endl;
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

void BinaryFileAdapter::SaveBodyProperties(double time, list<Body* >& bodyList, output_type_t type)
{
	switch(type)
	{
		case OUTPUT_TYPE_BINARY:
		{
			string constPropPath = output->GetPath(output->constantProperties);
			ofstream constPropWriter;
			static bool firstcallC = true;

			if (firstcallC) {
				constPropWriter.open(constPropPath.c_str(), ios::out | ios::binary);
				firstcallC = false;
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
			static bool firstcallV = true;

			if (firstcallV) {
				varPropWriter.open(varPropPath.c_str(), ios::out | ios::binary);
				firstcallV = false;
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
				SaveVariableProperty(varPropWriter, *it, time, type);
			}
			constPropWriter.close();
			varPropWriter.close();
			break;
		}
		case OUTPUT_TYPE_TEXT:
		{
			string constPropPath = output->GetPath(output->GetFilenameWithoutExt(output->constantProperties) + ".txt");
			ofstream constPropWriter;
			static bool firstcallC = true;

			if (firstcallC) {
				constPropWriter.open(constPropPath.c_str(), ios::out);
				firstcallC = false;
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
			static bool firstcallV = true;

			if (firstcallV) {
				varPropWriter.open(varPropPath.c_str(), ios::out);
				firstcallV = false;
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
				SaveVariableProperty(varPropWriter, *it, time, type);
			}
			constPropWriter.close();
			varPropWriter.close();
			break;
		}
	}

	
}

void BinaryFileAdapter::SaveConstantProperty(Body* body, output_type_t type)
{
	switch(type)
	{
		case OUTPUT_TYPE_BINARY:
		{
			string constPropPath = output->GetPath(output->constantProperties);
			ofstream constPropWriter;
			static bool firstcall = true;

			if (firstcall) {
				constPropWriter.open(constPropPath.c_str(), ios::out | ios::binary);
				firstcall = false;
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
			static bool firstcall = true;

			if (firstcall) {
				constPropWriter.open(constPropPath.c_str(), ios::out);
				firstcall = false;
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

void BinaryFileAdapter::SaveVariableProperty(Body* body, double time, output_type_t type)
{
	switch(type)
	{
		case OUTPUT_TYPE_BINARY:
		{
			string varPropPath = output->GetPath(output->variableProperties);
			ofstream varPropWriter;
			static bool firstcall = true;

			if (firstcall) {
				varPropWriter.open(varPropPath.c_str(), ios::out | ios::binary);
				firstcall = false;
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
			SaveVariableProperty(varPropWriter, body, time, type);
			varPropWriter.close();
			break;
		}
		case OUTPUT_TYPE_TEXT:
		{
			string varPropPath = output->GetPath(output->GetFilenameWithoutExt(output->variableProperties) + ".txt");
			ofstream varPropWriter;
			static bool firstcall = true;

			if (firstcall) {
				varPropWriter.open(varPropPath.c_str(), ios::out);
				firstcall = false;
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
			SaveVariableProperty(varPropWriter, body, time, type);
			varPropWriter.close();
			break;
		}
	}
	
	
}

void BinaryFileAdapter::SaveVariableProperty(ofstream& writer, Body* body, double time, output_type_t type)
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
					static bool firstcall = true;

					if (firstcall) {
						compPropWriter.open(compPropPath.c_str(), ios::out | ios::binary);
						firstcall = false;
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
					static bool firstcall = true;

					if (firstcall) {
						compPropWriter.open(compPropPath.c_str(), ios::out);
						firstcall = false;
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

void BinaryFileAdapter::SaveCollisionProperty(BodyData* bodyData, output_type_t type, int i, int j)
{
	switch(type)
	{
		case OUTPUT_TYPE_BINARY:
		{
			//TODO: implement
			break;
		}

		case OUTPUT_TYPE_TEXT:
		{

			//std::stringstream filename;
			//filename << "CollisionProperties" << i << '_' << j << ".txt";
			//string path = output->GetPath(filename.str());
			Phase phase1, phase2, phase12;
			OrbitalElement oe1, oe2, oe12;

			double *y1AC = new double[6];
			double *y2AC = new double[6];
			double *y12AC = new double[6];

			for (int k = 0; k < 6 ; k++) {
				y1AC[k] = bodyData->y0[6*i+k] - bodyData->y0[k];
				y2AC[k] = bodyData->y0[6*j+k] - bodyData->y0[k];

				y12AC[k] = bodyData->y0[6*i+k] - bodyData->y0[6*j+k];
			}
			Tools::ToPhase(y1AC, &phase1);
			Tools::ToPhase(y2AC, &phase2);
			double mu1 = Constants::Gauss2*(bodyData->mass[0] + bodyData->mass[i]);
			double mu2 = Constants::Gauss2*(bodyData->mass[0] + bodyData->mass[j]);
			Ephemeris::CalculateOrbitalElement(mu1,&phase1,&oe1);
			Ephemeris::CalculateOrbitalElement(mu2,&phase2,&oe2);

			Tools::ToPhase(y12AC, &phase12);
			double mu12 = Constants::Gauss2*(bodyData->mass[j] + bodyData->mass[i]);
			if (Ephemeris::CalculateOrbitalElement(mu12,&phase12,&oe12) == 0) {
				string pathtemp = output->GetPath("valami.txt");
				ofstream writertemp;
				writertemp.open(pathtemp.c_str(), ios::out | ios::app);
				writertemp << "valami" << endl;
			}

			delete[] y1AC;
			delete[] y2AC;
			delete[] y12AC;

			string path = output->GetPath("CollisionProperties.txt");
			ofstream writer;
			static bool firstcall = true;

			if (firstcall) {
				writer.open(path.c_str(), ios::out);
				//writer << setw(20) << "time [day]" << setw(20) << "stepsize [day]" << setw(20) << "id1" << setw(20) << "id2" << setw(20) << "idxofnn1" << setw(20) << "idxofnn2";
				//writer << setw(20) << "distofnn1 [AU]" << setw(20) << "distofnn2 [AU]" << setw(20) << "mass1 [Solar]" << setw(20) << "mass2 [Solar]";
				//writer << setw(20) << "radius1 [km]" << setw(20) << "radius2 [km]" << setw(20) << "dens1 [Solar/AU^3]" << setw(20) << "dens2 [Solar/AU^3]";
				//writer << setw(20) << "x01 [AU]" << setw(20) << "y01 [AU]" << setw(20) << "z01 [AU]" << setw(20) << "vx01 [AU/day]" << setw(20) << "vy01 [AU/day]" << setw(20) << "vz01 [AU/day]";
				//writer << setw(20) << "x02 [AU]" << setw(20) << "y02 [AU]" << setw(20) << "z02 [AU]" << setw(20) << "vx02 [AU/day]" << setw(20) << "vy02 [AU/day]" << setw(20) << "vz02 [AU/day]";
				////writer << setw(20) << "x1 [AU]" << setw(20) << "y1 [AU]" << setw(20) << "z1 [AU]" << setw(20) << "vx1 [AU/day]" << setw(20) << "vy1 [AU/day]" << setw(20) << "vz1 [AU/day]";
				////writer << setw(20) << "x2 [AU]" << setw(20) << "y2 [AU]" << setw(20) << "z2 [AU]" << setw(20) << "vx2 [AU/day]" << setw(20) << "vy2 [AU/day]" << setw(20) << "vz2 [AU/day]";
				//writer << setw(20) << "ax1 [AU/day^2]" << setw(20) << "ay1 [AU/day^2]" << setw(20) << "az1 [AU/day^2]" << setw(20) << "ax2 [AU/day^2]" << setw(20) << "ay2 [AU/day^2]" << setw(20) << "az2 [AU/day^2]";
				//writer << setw(20) << "errx1 [AU]" << setw(20) << "erry1 [AU]" << setw(20) << "errz1 [AU]" << setw(20) << "errvx1 [AU/day]" << setw(20) << "errvy1 [AU/day]" << setw(20) << "errvz1 [AU/day]";
				//writer << setw(20) << "errx2 [AU]" << setw(20) << "erry2 [AU]" << setw(20) << "errz2 [AU]" << setw(20) << "errvx2 [AU/day]" << setw(20) << "errvy2 [AU/day]" << setw(20) << "errvz2 [AU/day]";
				//writer << setw(20) << "mass [Solar]" << setw(20) << "bcx [AU]" << setw(20) << "bcy [AU]" << setw(20) << "bcz [AU]";
				//writer << setw(20) << "bcvx [AU/day]" << setw(20) << "bcvy [AU/day]" << setw(20) << "bcvz [AU/day]" << setw(20) << "bcr [AU]" << setw(20) << "bcv [AU/day]";
				//writer << setw(20) << "cx []" << setw(20) << "cy []" << setw(20) << "cz []" << setw(20) << "c []" << setw(20) << "kinen []" << setw(20) << "poten []" << setw(20) << "toten []";
				//writer << endl;

				firstcall = false;
			}
			else {
				writer.open(path.c_str(), ios::out | ios::app);
			}

			if (!writer) {
				_errMsg = "The file '" + path + "' could not opened!";
				Log(_errMsg, true);
				perror(_errMsg.c_str());
				exit(1);
			}
			
			
			writer << setw(20) << setprecision(10) << bodyData->time;
			writer << setw(20) << setprecision(10) << bodyData->h;
			writer << setw(20) << bodyData->id[i];
			writer << setw(20) << bodyData->id[j];
			//writer << setw(20) << bodyData->indexOfNN[i];
			//writer << setw(20) << bodyData->indexOfNN[j];
			writer << setw(20) << setprecision(10) << bodyData->distanceOfNN[i];
			//writer << setw(20) << setprecision(10) << bodyData->distanceOfNN[j];
			writer << setw(20) << setprecision(10) << bodyData->mass[i];
			writer << setw(20) << setprecision(10) << bodyData->mass[j];
			writer << setw(20) << setprecision(10) << bodyData->radius[i];
			writer << setw(20) << setprecision(10) << bodyData->radius[j];
			writer << setw(20) << setprecision(10) << bodyData->density[i];
			writer << setw(20) << setprecision(10) << bodyData->density[i];
			for (int k = 0; k < 6; k++) {
				writer << setw(20) << setprecision(10) << bodyData->y0[6*i+k];
			}
			writer << setw(20) << setprecision(10) << oe1.semiMajorAxis;
			writer << setw(20) << setprecision(10) << oe1.eccentricity;
			writer << setw(20) << setprecision(10) << oe1.inclination;
			writer << setw(20) << setprecision(10) << oe1.argumentOfPericenter;
			writer << setw(20) << setprecision(10) << oe1.longitudeOfNode;
			writer << setw(20) << setprecision(10) << oe1.meanAnomaly;
			for (int k = 0; k < 6; k++) {
				writer << setw(20) << setprecision(10) << bodyData->y0[6*j+k];
			}
			writer << setw(20) << setprecision(10) << oe2.semiMajorAxis;
			writer << setw(20) << setprecision(10) << oe2.eccentricity;
			writer << setw(20) << setprecision(10) << oe2.inclination;
			writer << setw(20) << setprecision(10) << oe2.argumentOfPericenter;
			writer << setw(20) << setprecision(10) << oe2.longitudeOfNode;
			writer << setw(20) << setprecision(10) << oe2.meanAnomaly;
			/*for (int k = 0; k < 6; k++) {
				writer << setw(20) << setprecision(10) << bodyData->y[6*i+k];
			}
			for (int k = 0; k < 6; k++) {
				writer << setw(20) << setprecision(10) << bodyData->y[6*j+k];
			}*/
			for (int k = 3; k < 6; k++) {
				writer << setw(20) << setprecision(10) << bodyData->accel[6*i+k];
			}
			for (int k = 3; k < 6; k++) {
				writer << setw(20) << setprecision(10) << bodyData->accel[6*j+k];
			}
			for (int k = 0; k < 6; k++) {
				writer << setw(20) << setprecision(10) << bodyData->error[6*i+k];
			}
			for (int k = 0; k < 6; k++) {
				writer << setw(20) << setprecision(10) << bodyData->error[6*j+k];
			}
			for (int k = 0; k < 16; k++) {
				writer << setw(20) << setprecision(10) << bodyData->integrals[k];
			}
			writer << endl;

			if (writer.bad()) {
				_errMsg = "An error occurred during writing the collision properties!";
				Log(_errMsg, true);
				perror(_errMsg.c_str());
				exit(1);
			}

			writer.close();

			break;
		}
	}
}

void BinaryFileAdapter::SaveElapsedTimes(double time, Counter counter, StopWatch timer1, StopWatch timer2, output_type_t type) {
	switch(type)
	{
		case OUTPUT_TYPE_BINARY:
		{
			break;
		}
		case OUTPUT_TYPE_TEXT:
		{
			string path = output->GetPath("ElapsedTimeIntSim.txt");
			ofstream writer;
			static bool firstcall = true;

			if (firstcall) {
				writer.open(path.c_str(), ios::out);
				firstcall = false;
			}
			else {
				writer.open(path.c_str(), ios::out | ios::app);
			}

			if (!writer) {
				_errMsg = "The file '" + path + "' could not opened!";
				Log(_errMsg, true);
				perror(_errMsg.c_str());
				exit(1);
			}

			writer << setw(15) << counter.succededStep;
			writer << setw(15) << time;
			writer << setw(15) << timer1.getElapsedTime();
			writer << setw(15) << timer2.getElapsedTime();
			writer << endl;
								
			if (writer.bad()) {
				_errMsg = "An error occurred during writing the composition property!";
				Log(_errMsg, true);
				perror(_errMsg.c_str());
				exit(1);
			}
			break;
		}
	}
}

void BinaryFileAdapter::SaveElapsedTimes(double time, StopWatch timer, output_type_t type) {
	switch(type)
	{
		case OUTPUT_TYPE_BINARY:
		{
			break;
		}
		case OUTPUT_TYPE_TEXT:
		{
			string path = output->GetPath("ElapsedTimeStep.txt");
			ofstream writer;
			static bool firstcall = true;

			if (firstcall) {
				writer.open(path.c_str(), ios::out);
				firstcall = false;
			}
			else {
				writer.open(path.c_str(), ios::out | ios::app);
			}

			if (!writer) {
				_errMsg = "The file '" + path + "' could not opened!";
				Log(_errMsg, true);
				perror(_errMsg.c_str());
				exit(1);
			}

			writer << setw(15) << time;
			writer << setw(15) << timer.getElapsedTime();
			writer << endl;
								
			if (writer.bad()) {
				_errMsg = "An error occurred during writing the composition property!";
				Log(_errMsg, true);
				perror(_errMsg.c_str());
				exit(1);
			}
			break;
		}
	}
}

void BinaryFileAdapter::SaveElapsedTimes(double time, StopWatch *timer, output_type_t type) {
	switch(type)
	{
		case OUTPUT_TYPE_BINARY:
		{
			break;
		}
		case OUTPUT_TYPE_TEXT:
		{
			string path = output->GetPath("ElapsedTimeForce.txt");
			ofstream writer;
			static bool firstcall = true;

			if (firstcall) {
				writer.open(path.c_str(), ios::out);
				firstcall = false;
			}
			else {
				writer.open(path.c_str(), ios::out | ios::app);
			}

			if (!writer) {
				_errMsg = "The file '" + path + "' could not opened!";
				Log(_errMsg, true);
				perror(_errMsg.c_str());
				exit(1);
			}

			writer << setw(15) << time;
			for (int i = 0; i < 13; i++) {
				writer << setw(15) << timer[i].getElapsedTime();
			}
			writer << endl;
								
			if (writer.bad()) {
				_errMsg = "An error occurred during writing the composition property!";
				Log(_errMsg, true);
				perror(_errMsg.c_str());
				exit(1);
			}
			break;
		}
	}
}
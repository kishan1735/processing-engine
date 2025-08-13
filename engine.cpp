#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <map>
#include "zmq.hpp"

struct AggregateData
{
    int min_score = INT32_MAX;
    int max_score = INT32_MIN;
    long long total_score_sum = 0;
    int record_count = 0;
};

using namespace std;

vector<string> parse_file_list(const string &file_list_str)
{
    vector<string> files;
    stringstream ss(file_list_str);
    string file;

    while (getline(ss, file, ','))
    {
        files.push_back(file);
    }
    return files;
}

string process_files(const vector<string> &files)
{
    map<int, AggregateData> yearly_results;

    for (const auto &file_path : files)
    {
        ifstream input_file(file_path);
        if (!input_file.is_open())
        {
            cerr << "Engine could not open file: " << file_path << endl;
            continue;
        }

        string line;
        while (getline(input_file, line))
        {
            stringstream ss(line);
            string student_id, batch_year_str, score_str;

            getline(ss, student_id, ',');
            getline(ss, batch_year_str, ',');
            getline(ss, score_str, ',');

            try
            {
                int year = stoi(batch_year_str);
                int score = stoi(score_str);

                yearly_results[year].min_score = min(yearly_results[year].min_score, score);
                yearly_results[year].max_score = max(yearly_results[year].max_score, score);
                yearly_results[year].total_score_sum += score;
                yearly_results[year].record_count++;
            }
            catch (const invalid_argument &e)
            {
                cerr << "Line is malformed" << endl;
            }
        }
    }
    string result_str;
    for (const auto &pair : yearly_results)
    {
        result_str += to_string(pair.first) + "," +
                      to_string(pair.second.min_score) + "," +
                      to_string(pair.second.max_score) + "," +
                      to_string(pair.second.total_score_sum) + "," +
                      to_string(pair.second.record_count) + ";";
    }
    return result_str;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        cerr << "Invalid arguements" << endl;
        return 1;
    }

    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_REP);

    string address = "tcp://*:" + string(argv[1]);
    socket.bind(address);
    cout << "Engine started on port " << argv[1] << "..." << endl;

    zmq::message_t request;
    socket.recv(request, zmq::recv_flags::none);
    string received_str = request.to_string();

    vector<string> files_to_process = parse_file_list(received_str);
    cout << "Engine on port " << argv[1] << " received " << files_to_process.size() << " files to process." << endl;

    string results = process_files(files_to_process);
    socket.send(zmq::buffer(results), zmq::send_flags::none);

    return 0;
}

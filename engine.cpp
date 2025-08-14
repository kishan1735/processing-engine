#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <algorithm>
#include "zmq.hpp"

struct AggregateData
{
    int min_score = INT32_MAX;
    int max_score = INT32_MIN;
    long long total_score_sum = 0;
    int record_count = 0;
};

using namespace std;

inline int fast_stoi(char *&p)
{
    int x = 0;
    while (*p >= '0' && *p <= '9')
    {
        x = (x * 10) + (*p - '0');
        ++p;
    }
    return x;
}

vector<string> parse_files(const string &file_list_str)
{
    vector<string> files;
    string current;
    for (char c : file_list_str)
    {
        if (c == ',')
        {
            files.push_back(current);
            current.clear();
        }
        else
        {
            current += c;
        }
    }
    if (!current.empty())
    {
        files.push_back(current);
    }
    return files;
}

string process_files(const vector<string> &files)
{
    unordered_map<int, AggregateData> yearly_results;
    vector<char> buffer(2 * 1024 * 1024);
    string left;

    for (const auto &file_path : files)
    {
        ifstream input_file(file_path, ios::binary);
        left.clear();
        while (true)
        {
            input_file.read(buffer.data(), buffer.size());
            streamsize bytes_read = input_file.gcount();
            if (bytes_read == 0)
                break;

            char *p = buffer.data();
            char *end = p + bytes_read;
            while (p < end)
            {
                while (p < end && *p != ',')
                    p++;
                if (p == end)
                    break;
                p++;

                int year = fast_stoi(p);
                if (p == end)
                    break;
                p++;
                int score = fast_stoi(p);

                auto &data = yearly_results[year];
                data.min_score = min(data.min_score, score);
                data.max_score = max(data.max_score, score);
                data.total_score_sum += score;
                data.record_count++;

                while (p < end && *p != '\n')
                    p++;
                if (p == end)
                    break;
                p++;
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

    vector<string> files_to_process = parse_files(received_str);

    string results = process_files(files_to_process);

    socket.send(zmq::buffer(results), zmq::send_flags::none);

    return 0;
}

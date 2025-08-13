#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <filesystem>
#include <numeric>
#include <map>
#include <mutex>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <cmath>

#include "zmq.hpp"

using namespace std;

struct AggregateData
{
    int min_score = INT32_MAX;
    int max_score = INT32_MIN;
    long long total_score_sum = 0ll;
    int record_count = 0;
};

unordered_map<int, AggregateData> final_results;
mutex results_mutex;

void parse_and_merge_results(const string &partial)
{
    stringstream ss(partial);
    string record;

    while (getline(ss, record, ';'))
    {
        if (record.empty())
            continue;

        stringstream record_ss(record);
        string year_str, min_str, max_str, sum_str, count_str;

        getline(record_ss, year_str, ',');
        getline(record_ss, min_str, ',');
        getline(record_ss, max_str, ',');
        getline(record_ss, sum_str, ',');
        getline(record_ss, count_str, ',');

        try
        {
            int year = stoi(year_str);
            int min_val = stoi(min_str);
            int max_val = stoi(max_str);
            long long sum_val = stoll(sum_str);
            int count_val = stoi(count_str);

            lock_guard<mutex> lock(results_mutex);

            auto &data = final_results[year];
            data.min_score = min(data.min_score, min_val);
            data.max_score = max(data.max_score, max_val);
            data.total_score_sum += sum_val;
            data.record_count += count_val;
        }
        catch (const invalid_argument &e)
        {
            cerr << "Error parsing record: " << record << endl;
        }
    }
}

void communicate(string port, string file_list)
{
    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_REQ);

    string address = "tcp://localhost:" + port;
    socket.connect(address);

    socket.send(zmq::buffer(file_list), zmq::send_flags::none);

    zmq::message_t reply;
    if (socket.recv(reply, zmq::recv_flags::none))
    {
        string partial = reply.to_string();
        parse_and_merge_results(partial);
    }
    else
    {
        cerr << "Error: Failed to receive reply from engine on port " << port << endl;
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        cerr << "Improper format ./driver <port1> <port2> <port3> should be the format" << endl;
        return 1;
    }

    vector<string> ports;
    for (int i = 1; i < argc; i++)
    {
        ports.push_back(argv[i]);
    }
    const int n_engines = ports.size();

    const filesystem::path data_dir{"./sample_dataset/student_scores"};
    vector<string> all_files;
    try
    {
        for (const auto &entry : filesystem::directory_iterator(data_dir))
        {
            if (entry.is_regular_file() && entry.path().filename() != "schema.csv")
            {
                all_files.push_back(entry.path().string());
            }
        }
    }
    catch (const filesystem::filesystem_error &e)
    {
        cerr << "Error accessing directory: " << e.what() << endl;
        return 1;
    }

    if (all_files.empty())
    {
        cerr << "No data files found in " << data_dir.string() << ". Exiting." << endl;
        return 1;
    }

    vector<vector<string>> engine_files(n_engines);
    for (size_t i = 0; i < all_files.size(); i++)
    {
        engine_files[i % n_engines].push_back(all_files[i]);
    }

    vector<thread> threads;
    for (int i = 0; i < n_engines; i++)
    {
        if (engine_files[i].empty())
            continue;

        string file_list_str = accumulate(next(engine_files[i].begin()), engine_files[i].end(), engine_files[i][0],
                                          [](const string &a, const string &b)
                                          { return a + ',' + b; });

        threads.emplace_back(communicate, ports[i], file_list_str);
    }

    for (auto &t : threads)
    {
        t.join();
    }

    cout << "All engines finished. Sorting and compiling results..." << endl;

    vector<pair<int, AggregateData>> sorted_results;
    for (const auto &pair : final_results)
    {
        sorted_results.push_back(pair);
    }

    sort(sorted_results.begin(), sorted_results.end(),
         [](const auto &a, const auto &b)
         {
             return a.first < b.first;
         });

    ofstream output_file("output.txt");

    for (const auto &pair : sorted_results)
    {
        const int year = pair.first;
        const auto &data = pair.second;
        long long avg_score = 0;
        if (data.record_count > 0)
        {
            avg_score = floor((1.0 * data.total_score_sum) / data.record_count);
        }

        output_file << year << ","
                    << data.min_score << ","
                    << data.max_score << ","
                    << avg_score << endl;
    }

    cout << "Results have been written to output.txt" << endl;

    return 0;
}

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <stdexcept>
#include <iomanip>
#include <limits> // For numeric limits

// Class representing a candlestick
class Candlestick
{
public:
    std::string date;
    double open;
    double high;
    double low;
    double close;

    Candlestick(const std::string &date, double open, double high, double low, double close)
        : date(date), open(open), high(high), low(low), close(close) {}
};

// CSVReader class to read and parse the CSV file
class CSVReader
{
public:
    std::unordered_map<std::string, std::vector<std::pair<std::string, double>>>
    readCSV(const std::string &csvFile, const std::string &countryCode)
    {
        std::unordered_map<std::string, std::vector<std::pair<std::string, double>>> weatherData;

        std::ifstream file(csvFile);
        std::string line;

        if (!file.is_open())
        {
            throw std::runtime_error("Error: Failed to open file: " + csvFile);
        }

        // Read header to locate the target column for the country
        std::getline(file, line);
        std::vector<std::string> headers = tokenise(line, ',');

        int countryColIndex = -1;
        for (size_t i = 0; i < headers.size(); ++i)
        {
            if (headers[i] == countryCode)
            {
                countryColIndex = static_cast<int>(i);
                break;
            }
        }

        if (countryColIndex == -1)
        {
            throw std::runtime_error("Error: Country code not found in the dataset.");
        }

        // Parse data
        while (std::getline(file, line))
        {
            std::vector<std::string> tokens = tokenise(line, ',');
            if (tokens.size() > static_cast<size_t>(countryColIndex))
            {
                try
                {
                    std::string date = tokens[0]; // Assuming the date is in the first column
                    double temperature = std::stod(tokens[countryColIndex]);
                    weatherData[date.substr(0, 4)].push_back(std::make_pair(date, temperature)); // Group by year
                }
                catch (const std::exception &e)
                {
                    std::cerr << "CSVReader::readCSV - Invalid data: " << e.what() << std::endl;
                }
            }
        }

        return weatherData;
    }

private:
    std::vector<std::string> tokenise(const std::string &csvLine, char separator)
    {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream stream(csvLine);

        while (std::getline(stream, token, separator))
        {
            tokens.push_back(token);
        }

        return tokens;
    }
};

// Class to calculate candlestick data
class CandlestickCalculator
{
public:
    std::vector<Candlestick> computeCandlesticks(
        const std::unordered_map<std::string, std::vector<std::pair<std::string, double>>> &weatherData)
    {
        std::vector<Candlestick> candlesticks;
        double previousClose = 0.0;

        for (std::unordered_map<std::string, std::vector<std::pair<std::string, double>>>::const_iterator it = weatherData.begin(); it != weatherData.end(); ++it)
        {
            std::string year = it->first;
            const std::vector<std::pair<std::string, double>> &data = it->second;

            double high = std::numeric_limits<double>::lowest();
            double low = std::numeric_limits<double>::max();
            double totalTemperature = 0.0;

            for (std::vector<std::pair<std::string, double>>::const_iterator jt = data.begin(); jt != data.end(); ++jt)
            {
                double temp = jt->second;
                high = std::max(high, temp);
                low = std::min(low, temp);
                totalTemperature += temp;
            }

            double close = totalTemperature / data.size();
            double open = previousClose;
            previousClose = close;

            candlesticks.push_back(Candlestick(year, open, high, low, close));
        }

        return candlesticks;
    }
};

// Main function
int main()
{
    std::cout << "Program begins" << std::endl;
    try
    {
        const std::string csvFilename = "weather_data.csv";
        const std::string countryCode = "GB_temperature"; // Replace with your column name

        // Read and process the data
        std::cout << "Reading CSV" << std::endl;
        CSVReader reader;
        std::unordered_map<std::string, std::vector<std::pair<std::string, double>>> weatherData = reader.readCSV(csvFilename, countryCode);
        std::cout << "Finished reading CSV" << std::endl;

        // Compute candlestick data
        CandlestickCalculator calculator;
        std::vector<Candlestick> candlesticks = calculator.computeCandlesticks(weatherData);

        // Display candlestick data
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Date\tOpen\tHigh\tLow\tClose" << std::endl;
        for (std::vector<Candlestick>::const_iterator it = candlesticks.begin(); it != candlesticks.end(); ++it)
        {
            std::cout << it->date << "\t" << it->open << "\t" << it->high << "\t" << it->low << "\t" << it->close << std::endl;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}

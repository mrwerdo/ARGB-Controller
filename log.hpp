#ifndef LOG_HPP
#define LOG_HPP

enum delim { newline, comma, none };

// #ifdef AVR

template <typename T> void log(String field, T value, delim d = delim::comma)
{
    Serial.print(field);
    Serial.print(F(": "));
    Serial.print(value);
    switch (d) {
    case delim::newline:
        Serial.println();
        break;
    case delim::comma:
        Serial.print(", ");
        break;
    default:
        break;
    }
}

// #else

// template <typename T> void log(String field, T value, delim d = delim::comma)
// {
//     using namespace std;
//     cout << field << ": " << value;
//     switch (d) {
//     case delim::newline:
//         cout << endl;
//         break;
//     case delim::comma:
//         cout << ", ";
//         break;
//     default:
//         break;
//     }
// }

// #endif /* #ifdef AVR */

#endif /* #ifndef LOG_HPP */
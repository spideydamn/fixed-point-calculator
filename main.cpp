#include <string>
#include <cstdint>
#include <iostream>
#include <cstring>

enum class Type;
enum class Operation;
enum class Sign;

void Parse(int argc, char** argv);

std::pair<uint8_t, uint8_t> CheckFormat(char* arg);
Type CheckRoundingValue(char* arg);
Operation CheckOperation(char* arg);
uint64_t CheckHexadecimalNumber(char* arg);

uint64_t Round(uint64_t number, Sign sign, Type type, uint8_t real_part_size);

uint64_t RoundTowardInfinity(uint64_t number, uint8_t real_part_size);
uint64_t RoundTowardNegInfinity(uint64_t number, uint8_t real_part_size);
uint64_t RoundNearestEven(uint64_t number, uint8_t real_part_size);

uint64_t Calculate(std::pair<uint8_t, uint8_t>& format, uint64_t first_number, Operation operation, uint64_t second_number, Type type);

uint64_t Add(uint64_t first_number, uint64_t second_number);
uint64_t Subtract(uint64_t first_number, uint64_t second_number);
uint64_t Multiply(std::pair<uint8_t, uint8_t>& format, uint64_t first_number, uint64_t second_number, Type type);
uint64_t Divide(std::pair<uint8_t, uint8_t>& format, uint64_t first_number, uint64_t second_number, Type type);

uint64_t Pow(uint64_t base, uint64_t degree);

bool CheckBit(uint64_t number, uint8_t n);
uint64_t LeaveNLowBits(uint64_t number, uint8_t n);
uint64_t ClearBit(uint64_t number, uint64_t n);
uint64_t ClearNLowBits(uint64_t number, uint8_t n);

const uint8_t kPrecision = 3;

enum class Type {
    kTowardZero,
    kNearestEven,
    kTowardInfinity,
    kTowardNegInfinity
};

enum class Operation {
    kAddition,
    kSubtraction,
    kMultiplication,
    kDivision
};

enum class Sign {
    kPositive,
    kNegative
};

void Parse(int argc, char **argv) {
    if (argc != 4 && argc != 6) {
        std::cerr << "Wrong number of arguments" << std::endl;
        exit(1);
    }

    std::pair<uint8_t, uint8_t> format = CheckFormat(argv[1]);
    Type type = CheckRoundingValue(argv[2]);
    uint64_t first_number = LeaveNLowBits(CheckHexadecimalNumber(argv[3]), format.first + format.second);

    uint64_t result;
    if (argc == 4) {
        result = first_number;
    } else {
        Operation operation = CheckOperation(argv[4]);
        uint64_t second_number = LeaveNLowBits(CheckHexadecimalNumber(argv[5]), format.first + format.second);

        result = Calculate(format, first_number, operation, second_number, type);
    }
    result = LeaveNLowBits(result, format.first + format.second);

    uint64_t integer_part = ClearNLowBits(result, format.second);
    uint64_t real_part = LeaveNLowBits(result, format.second);
    Sign sign = Sign::kPositive;

    if (CheckBit(result, format.first + format.second - 1)) {
        sign = Sign::kNegative;
        integer_part = LeaveNLowBits(-integer_part, format.first);
        if (real_part > 0) {
            --integer_part;
            real_part = LeaveNLowBits(-real_part, format.second);
        }
    }

    real_part = Round(real_part * Pow(10, 3), sign, type, format.second);

    if (real_part == Pow(10, kPrecision)) {
        real_part = 0;
        ++integer_part;
        if (sign == Sign::kPositive && integer_part == ((uint64_t)1 << (format.first - 1))) {
            sign = Sign::kNegative;
        }
    } else if (real_part == 0 && integer_part == 0) {
        sign = Sign::kPositive;
    }

    if (sign == Sign::kNegative) {
        std::cout << '-';
    }
    std::cout << integer_part << '.';
    for (uint64_t i = (real_part == 0) ? 10 : real_part * 10; i < Pow(10, kPrecision); i *= 10) {
        std::cout << 0;
    }
    std::cout << real_part;
}

std::pair<uint8_t, uint8_t> CheckFormat(char *arg) {
    std::pair<uint8_t, uint8_t> format = std::make_pair(0, 0);
    size_t len_arg = strlen(arg);
    bool was_point = false;
    for (int i = 0; i < len_arg; ++i) {
        if (!isdigit(arg[i])) {
            if (arg[i] == '.' && !was_point) {
                was_point = true;
            } else {
                std::cerr << "Invalid format" << std::endl;
                exit(1);
            }
        } else {
            if (!was_point) {
                format.first = format.first * 10 + (arg[i] - '0');
            } else {
                format.second = format.second * 10 + (arg[i] - '0');
            }
        }
    }
    return format;
}

Type CheckRoundingValue(char *arg) {
    size_t len_arg = strlen(arg);
    if (len_arg != 1) {
        std::cerr << "Invalid rounding value" << std::endl;
        exit(1);
    }
    switch (arg[0]) {
        case '0':
            return Type::kTowardZero;
        case '1':
            return Type::kNearestEven;
        case '2':
            return Type::kTowardInfinity;
        case '3':
            return Type::kTowardNegInfinity;
        default:
            std::cerr << "Invalid rounding value" << std::endl;
            exit(1);
    }
}

Operation CheckOperation(char *arg) {
    size_t len_arg = strlen(arg);
    if (len_arg != 1) {
        std::cerr << "Invalid operation sign" << std::endl;
        exit(1);
    }
    switch (arg[0]) {
        case '+':
            return Operation::kAddition;
        case '-':
            return Operation::kSubtraction;
        case '*':
            return Operation::kMultiplication;
        case '/':
            return Operation::kDivision;
        default:
            std::cerr << "Invalid operation sign" << std::endl;
            exit(1);
    }
}

uint64_t CheckHexadecimalNumber(char *arg) {
    uint64_t number = 0;
    size_t len_arg = strlen(arg);
    if (arg[0] != '0' || arg[1] != 'x' || len_arg <= 2) {
        std::cerr << "Invalid hexadecimal number argument" << std::endl;
        exit(1);
    }
    for (int i = 2; i < len_arg; ++i) {
        if (arg[i] >= '0' && arg[i] <= '9') {
            number = number * 16 + (arg[i] - '0');
        } else if (arg[i] >= 'a' && arg[i] <= 'f') {
            number = number * 16 + (10 + arg[i] - 'a');
        } else if (arg[i] >= 'A' && arg[i] <= 'F') {
            number = number * 16 + (10 + arg[i] - 'A');
        } else {
            std::cerr << "Invalid hexadecimal number argument" << std::endl;
            exit(1);
        }
    }
    return number;
}


uint64_t Round(uint64_t number, Sign sign, Type type, uint8_t real_part_size) {
    switch (type) {
        case Type::kTowardZero:
            return RoundTowardNegInfinity(number, real_part_size);
        case Type::kNearestEven:
            return RoundNearestEven(number, real_part_size);
        case Type::kTowardInfinity:
            switch (sign) {
                case Sign::kPositive:
                    return RoundTowardInfinity(number, real_part_size);
                case Sign::kNegative:
                    return RoundTowardNegInfinity(number, real_part_size);
            }
            break;
        case Type::kTowardNegInfinity:
            switch (sign) {
                case Sign::kPositive:
                    return RoundTowardNegInfinity(number, real_part_size);
                case Sign::kNegative:
                    return RoundTowardInfinity(number, real_part_size);
            }
            break;
    }
    return 0;
}

uint64_t RoundNearestEven(uint64_t number, uint8_t real_part_size) {
    if (LeaveNLowBits(number, real_part_size) == ((uint64_t)1 << (real_part_size - 1))) {
        if (CheckBit(number, real_part_size)) {
            return RoundTowardInfinity(number, real_part_size);
        }
        return RoundTowardNegInfinity(number, real_part_size);
    } else if (LeaveNLowBits(number, real_part_size) > ((uint64_t)1 << (real_part_size - 1))) {
        return RoundTowardInfinity(number, real_part_size);
    }
    return RoundTowardNegInfinity(number, real_part_size);
}

uint64_t RoundTowardInfinity(uint64_t number, uint8_t real_part_size) {
    if (LeaveNLowBits(number, real_part_size) > 0) {
        return ClearNLowBits(number, real_part_size) + 1;
    }
    return ClearNLowBits(number, real_part_size);
}

uint64_t RoundTowardNegInfinity(uint64_t number, uint8_t real_part_size) {
    return ClearNLowBits(number, real_part_size);
}

uint64_t Calculate(std::pair<uint8_t, uint8_t>& format, uint64_t first_number, Operation operation, uint64_t second_number, Type type) {
    switch (operation) {
        case Operation::kAddition:
            return Add(first_number, second_number);
        case Operation::kSubtraction:
            return Subtract(first_number, second_number);
        case Operation::kMultiplication:
            return Multiply(format, first_number, second_number, type);
        case Operation::kDivision:
            return Divide(format, first_number, second_number, type);
    }

    return 0;
}

uint64_t Add(uint64_t first_number, uint64_t second_number) {
    return first_number + second_number;
}

uint64_t Subtract(uint64_t first_number, uint64_t second_number) {
    return first_number - second_number;
}

uint64_t Multiply(std::pair<uint8_t, uint8_t>& format, uint64_t first_number, uint64_t second_number, Type type) {
    Sign first_sign = Sign::kPositive;
    Sign second_sign = Sign::kPositive;

    if (CheckBit(first_number, format.first + format.second - 1)) {
        first_sign = Sign::kNegative;
        first_number = LeaveNLowBits(-first_number, format.first + format.second);
    }
    if (CheckBit(second_number, format.first + format.second - 1)) {
        second_sign = Sign::kNegative;
        second_number = LeaveNLowBits(-second_number, format.first + format.second);
    }

    if (first_sign != second_sign) {
        return -Round(first_number * second_number, Sign::kNegative, type, format.second);
    }

    return Round(first_number * second_number, Sign::kPositive, type, format.second);
}

uint64_t Divide(std::pair<uint8_t, uint8_t>& format, uint64_t first_number, uint64_t second_number, Type type) {
    if (second_number == 0) {
        std::cerr << "division by zero" << std::endl;
        exit(0);
    }
    Sign first_sign = Sign::kPositive;
    Sign second_sign = Sign::kPositive;

    if (CheckBit(first_number, format.first + format.second - 1)) {
        first_sign = Sign::kNegative;
        first_number = LeaveNLowBits(-first_number, format.first + format.second);
    }
    if (CheckBit(second_number, format.first + format.second - 1)) {
        second_sign = Sign::kNegative;
        second_number = LeaveNLowBits(-second_number, format.first + format.second);
    }

    first_number <<= (65 - format.first - format.second);

    if (first_sign != second_sign) {
        return -Round(first_number / second_number, Sign::kNegative, type, (65 - format.first - 2 * format.second));
    }

    return Round(first_number / second_number, Sign::kPositive, type, (65 - format.first - 2 * format.second));
}

uint64_t Pow(uint64_t base, uint64_t degree) {
    uint64_t ans = 1;
    for (uint64_t i = 0; i < degree; ++i) {
        ans *= base;
    }
    return ans;
}

bool CheckBit(uint64_t number, uint8_t n) {
    return (number >> n) & (uint64_t) 1;
}

uint64_t LeaveNLowBits(uint64_t number, uint8_t n) {
    for (int i = n; i < 64; ++i) {
        number = ClearBit(number, i);
    }
    return number;
}

uint64_t ClearNLowBits(uint64_t number, uint8_t n) {
    return number >> n;
}

uint64_t ClearBit(uint64_t number, uint64_t n) {
    return number & ~((uint64_t)1 << n);
}

int _CRT_glob = 0;

int main(int argc, char** argv) {
    Parse(argc, argv);
}
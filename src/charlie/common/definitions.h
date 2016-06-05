#ifndef CHARLIE_COMMON_DEFINITIONS_H
#define CHARLIE_COMMON_DEFINITIONS_H

#define BYTECODE_VERSION 1

#define FRIEND_TEST(test_case_name, test_name)\
friend class test_case_name##_##test_name##_Test

#endif // !CHARLIE_COMMON_DEFINITIONS_H
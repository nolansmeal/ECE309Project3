// tests/p3/test_p3.cpp — YOUR P3 TEST SUITE.
//
// The five tests below are worked examples. Keep them, and add your own
// until you have AT LEAST 15 (spec §6 lists what must be covered).
// Name every test <area>_<what>: reg_ for the registry, ws_ for the
// workspace, calc_ for the calculator.
//
// Build & run:   cmake --build build && ./build/test_p3
// One area:      ./build/test_p3 reg_
// One test:      ./build/test_p3 =reg_add_and_find

#include "../mini_test.h"
#include "../test_helpers.h"

#include "tools/calculator.h"
#include "tools/tool_registry.h"
#include "tools/workspace.h"

#include <cstdio>
#include <memory>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

// A tool whose name you choose — use it to fill the registry in tests.
class NamedTool : public Tool {
public:
    explicit NamedTool(std::string n)
        : n_(std::move(n)), schema_(JsonValue::object()) {}

    std::string name() const override { return n_; }
    std::string description() const override { return "test tool"; }
    const JsonValue& schema() const override { return schema_; }

    ToolResult invoke(const JsonValue&, ToolContext&) override {
        return ToolResult::success(n_);
    }

private:
    std::string n_;
    JsonValue schema_;
};

// Tokenize with the provided tokenizer, then run YOUR calc_evaluate.
bool eval(const std::string& expr, double* value, std::string* error) {
    std::vector<CalcToken> tokens;

    if (!calc_tokenize(expr, &tokens, error)) {
        return false;
    }

    return calc_evaluate(tokens, value, error);
}

} // namespace

// ------------------------------------------------------------- examples

// Test 1: Verify that a tool can be added and found
TEST(reg_add_and_find) {
    ToolRegistry r;

    auto t = std::make_unique<NamedTool>("alpha");
    Tool* raw = t.get();

    REQUIRE(r.add(std::move(t)));

    CHECK(r.size() == 1);
    CHECK(r.find("alpha") == raw);
    CHECK(r.find("beta") == nullptr);
}

// Test 2: Verify that duplicate tools are rejected
TEST(reg_duplicate_keeps_original) {
    ToolRegistry r;

    auto first = std::make_unique<NamedTool>("alpha");
    Tool* original = first.get();

    REQUIRE(r.add(std::move(first)));

    CHECK(!r.add(std::make_unique<NamedTool>("alpha")));
    CHECK(r.find("alpha") == original);
    CHECK(r.size() == 1);
}

// Test 3: Verify that paths cannot escape the workspace
TEST(ws_rejects_parent_escape) {
    TempDir d;
    Workspace ws(d.path());

    fs::path out;
    std::string reason;

    REQUIRE(ws.resolve("notes/todo.txt", &out, &reason));

    CHECK(out == ws.root() / "notes" / "todo.txt");

    CHECK(!ws.resolve("../etc/passwd", &out, &reason));
    CHECK(!reason.empty());

    // Additional traversal cases
    CHECK(!ws.resolve("folder/../../outside.txt", &out, &reason));
    CHECK(!ws.resolve("../../outside.txt", &out, &reason));
}

// Test 4: Verify multiplication has higher precedence than addition
TEST(calc_precedence) {
    double v = 0;
    std::string err;

    REQUIRE(eval("2 + 3 * 4", &v, &err));
    CHECK(v == 14);

    REQUIRE(eval("20 / 4 + 3", &v, &err));
    CHECK(v == 8);

    REQUIRE(eval("2 + 12 / 3", &v, &err));
    CHECK(v == 6);
}

// Test 5: Verify division by zero is rejected
TEST(calc_division_by_zero) {
    double v = 0;
    std::string err;

    CHECK(!eval("1 / 0", &v, &err));
    CHECK(err == "division by zero");

    err.clear();
    CHECK(!eval("10 / (5 - 5)", &v, &err));
    CHECK(err == "division by zero");
}

// ------------------------------------------------------------- your tests

// Test 6: Verify that null pointers cannot be added
TEST(reg_null_rejected) {
    ToolRegistry r;

    CHECK(!r.add(nullptr));
    CHECK(r.size() == 0);
}

// Test 7: Verify that for_each visits every tool
TEST(reg_for_each_visits_each_once) {
    ToolRegistry r;

    REQUIRE(r.add(std::make_unique<NamedTool>("alpha")));
    REQUIRE(r.add(std::make_unique<NamedTool>("beta")));
    REQUIRE(r.add(std::make_unique<NamedTool>("gamma")));

    int count = 0;

    r.for_each([&](const Tool& tool) {
        CHECK(!tool.name().empty());
        count++;
    });

    CHECK(count == 3);
}

// Test 8: Verify that pointers remain valid after resizing
TEST(reg_pointer_stability) {
    ToolRegistry r;

    auto first = std::make_unique<NamedTool>("original");
    Tool* original = first.get();

    REQUIRE(r.add(std::move(first)));

    for (int i = 0; i < 1000; i++) {
        REQUIRE(r.add(std::make_unique<NamedTool>(
            "tool_" + std::to_string(i))));
    }

    CHECK(r.find("original") == original);
    CHECK(r.size() == 1001);
}

// Test 9: Insert 10,000 tools and measure hash table performance
TEST(reg_10000_tools) {
    ToolRegistry r;

    // Save each original pointer to check stability after rehashing.
    std::vector<Tool*> original_pointers;
    original_pointers.reserve(10000);

    for (int i = 0; i < 10000; i++) {
        std::string name = "tool_" + std::to_string(i);

        auto tool = std::make_unique<NamedTool>(name);
        Tool* original = tool.get();

        REQUIRE(r.add(std::move(tool)));
        original_pointers.push_back(original);

        // Check the load factor after every insertion.
        CHECK(r.load_factor() <= 1.0);
    }

    CHECK(r.size() == 10000);
    CHECK(r.max_probe_length() <= 64);

    // Verify all tools are found at their original addresses.
    for (int i = 0; i < 10000; i++) {
        std::string name = "tool_" + std::to_string(i);

        CHECK(r.find(name) == original_pointers[i]);
    }

    std::printf("Buckets: %zu\n", r.bucket_count());
    std::printf("Load factor: %.4f\n", r.load_factor());
    std::printf("Maximum chain length: %zu\n",
                r.max_probe_length());
}

// Test 10: Compare FNV-1a against a weak hash function
TEST(reg_alternative_hash) {
    ToolRegistry r;

    // Use the same 10,000 names as the stress test.
    for (int i = 0; i < 10000; i++) {
        REQUIRE(r.add(std::make_unique<NamedTool>(
            "tool_" + std::to_string(i))));
    }

    // Weak hash: first character plus string length.
    std::vector<int> weak_buckets(r.bucket_count(), 0);

    for (int i = 0; i < 10000; i++) {
        std::string name = "tool_" + std::to_string(i);

        std::size_t weak_hash =
            static_cast<unsigned char>(name[0]) + name.size();

        std::size_t index = weak_hash % r.bucket_count();

        weak_buckets[index]++;
    }

    int weak_max = 0;

    for (int count : weak_buckets) {
        if (count > weak_max) {
            weak_max = count;
        }
    }

    std::printf("FNV-1a longest chain: %zu\n",
                r.max_probe_length());
    std::printf("Weak hash longest chain: %d\n", weak_max);

    CHECK(r.max_probe_length() <
          static_cast<std::size_t>(weak_max));
}

// Test 11: Verify that empty and absolute paths are rejected
TEST(ws_empty_path) {
    TempDir d;
    Workspace ws(d.path());

    fs::path out;
    std::string reason;

    CHECK(!ws.resolve("", &out, &reason));
    CHECK(!reason.empty());

    reason.clear();
    CHECK(!ws.resolve("/etc/passwd", &out, &reason));
    CHECK(!reason.empty());
}

// Test 12: Verify that paths containing null characters are rejected
TEST(ws_null_character) {
    TempDir d;
    Workspace ws(d.path());

    fs::path out;
    std::string reason;

    std::string bad_path("file\0.txt", 9);

    CHECK(!ws.resolve(bad_path, &out, &reason));
    CHECK(!reason.empty());
}

// Test 13: Verify that paths with .. are normalized correctly
TEST(ws_normalized_path) {
    TempDir d;
    Workspace ws(d.path());

    fs::path out;
    std::string reason;

    REQUIRE(ws.resolve("folder/../file.txt", &out, &reason));
    CHECK(out == ws.root() / "file.txt");

    REQUIRE(ws.resolve("a/b/../c.txt", &out, &reason));
    CHECK(out == ws.root() / "a" / "c.txt");

    REQUIRE(ws.resolve("./notes.txt", &out, &reason));
    CHECK(out.lexically_normal() ==
          (ws.root() / "notes.txt").lexically_normal());
}

// Test 14: Verify that subtraction evaluates left to right
TEST(calc_left_to_right) {
    double v = 0;
    std::string err;

    REQUIRE(eval("10 - 4 - 3", &v, &err));
    CHECK(v == 3);

    REQUIRE(eval("20 / 2 / 2", &v, &err));
    CHECK(v == 5);

    REQUIRE(eval("8 - 2 + 1", &v, &err));
    CHECK(v == 7);
}

// Test 15: Verify that parentheses override precedence
TEST(calc_parentheses) {
    double v = 0;
    std::string err;

    REQUIRE(eval("(2 + 3) * 4", &v, &err));
    CHECK(v == 20);

    REQUIRE(eval("2 * (3 + 4) - 5", &v, &err));
    CHECK(v == 9);

    REQUIRE(eval("((2 + 3) * (4 - 1))", &v, &err));
    CHECK(v == 15);
}

// Test 16: Verify that negative numbers work correctly
TEST(calc_negative_numbers) {
    double v = 0;
    std::string err;

    REQUIRE(eval("-5 + 3", &v, &err));
    CHECK(v == -2);

    REQUIRE(eval("-4 * 2", &v, &err));
    CHECK(v == -8);

    REQUIRE(eval("3 + -2", &v, &err));
    CHECK(v == 1);
}

int main(int argc, char** argv) {
    return mt::run_all(argc, argv);
}

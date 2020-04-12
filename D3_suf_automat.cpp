#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <cassert>
#include <random>
#include <algorithm>
#include <functional>
#include <unordered_map>
#include <memory>
#include <istream>

class SuffixAutomaton {
private:
    struct Node;
public:
    typedef Node NodeType;

    SuffixAutomaton() {
        root = std::make_shared<Node>(0, nullptr, 0);
        last = root;
    }

    ~SuffixAutomaton() {}

    std::shared_ptr<const Node> GetRoot() const {
        return root;
    }

    std::vector<char> GetNodeString(std::shared_ptr<const Node> node) const {
        std::vector<char> buffer;
        if (!node) {
            return buffer;
        }
        while (node->parent) {
            buffer.push_back(node->ch);
            node = node->parent;
        }
        return buffer;
    }

    void AddString(const std::string& s) {
        for (auto c : s) {
            AddChar(c);
        }
        MarkTerminal();
        FindCounts(root);
    }

    void AddChar(char c) {
        std::shared_ptr<Node> newNode = std::make_shared<Node>(last->length + 1, last, c);
        while (last && !last->canGo(c)) {
            last->children[c] = newNode;
            last = last->link;
        }

        if (!last) {
            last = newNode;
            last->link = root;
            return;
        }

        std::shared_ptr<Node> preCloned = last->children[c];
        std::shared_ptr<Node> cloned = Clone(preCloned, last->length, last, c);
        newNode->link = cloned;
        while (last && last->children[c] == preCloned) {
            last->children[c] = cloned;
            last = last->link;
        }
        last = newNode;
    }

    bool Find(const std::string& s) const {
        auto node = root;
        for (auto c : s) {
            if (node->canGo(c)) {
                node = node->go(c);
            } else {
                return false;
            }
        }
        return true;
    }

private:

    void FindCounts(const std::shared_ptr<Node>& node) {
        used[node] = true;
        for (auto child : node->children) {
            if (!used[child.second])
                FindCounts(child.second);
            node->count += child.second->count;
        }
        if (node->isTerminal) {
            node->count += 1;
        }
    }

    void MarkTerminal() const {
        auto node = last;
        while (node) {
            node->isTerminal = true;
            node = node->link;
        }
    }

    std::shared_ptr<Node> Clone(const std::shared_ptr<Node>& node, ssize_t length, const std::shared_ptr<Node>& parent, char c) const {
        std::shared_ptr<Node> cloned = std::make_shared<Node>(*node);
        cloned->length = length + 1;
        cloned->parent = parent;
        cloned->ch = c;
        node->link = cloned;
        return cloned;
    }

    struct Node {
        std::shared_ptr<Node> link;
        std::map<char, std::shared_ptr<Node>> children;
        ssize_t length;
        ssize_t count;
        bool isTerminal;
        std::shared_ptr<Node> parent;
        char ch;

        Node(ssize_t length_, const std::shared_ptr<Node>& parent_, char ch_) : length(length_), link(nullptr), count(0), isTerminal(false), parent(parent_), ch(ch_) {}

        ~Node() {}

        bool canGo(char c) const {
            return children.find(c) != children.end();
        }

        std::shared_ptr<Node> go(char c) {
            return children[c];
        }
    };

    std::shared_ptr<Node> last;
    std::shared_ptr<Node> root;

    std::unordered_map<std::shared_ptr<Node>, bool> used;
};

struct Answer {
    Answer(const std::vector<char>& refrenString_, ssize_t refrenValue_) :
        refrenString(refrenString_),
        refrenValue(refrenValue_)
    {}

    std::vector<char> refrenString;
    ssize_t refrenValue;
};

class RefrenFinder {
public:
    typedef SuffixAutomaton::NodeType Node;

    RefrenFinder(const SuffixAutomaton& automaton_) :
            automaton(automaton_),
            refren(-1),
            refrenNode(nullptr)
    {}

    ~RefrenFinder() = default;

    Answer GetRefren() {
        FindRefren(automaton.GetRoot());
        return {automaton.GetNodeString(refrenNode), refren};
    }

private:
    void FindRefren(const std::shared_ptr<const Node>& node) {
        used[node] = true;
        for (const std::pair<char, std::shared_ptr<const Node>>& child : node->children) {
            if (!used[child.second]) {
                FindRefren(child.second);
            }
        }
        if (node->length * node->count > refren) {
            refren = node->length * node->count;
            refrenNode = node;
        }
    }

    const SuffixAutomaton& automaton;
    ssize_t refren;
    std::shared_ptr<const Node> refrenNode;
    std::unordered_map<std::shared_ptr<const Node>, bool> used;
};

std::string GenerateString(size_t length) {
    const char alphabet[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g',
                             'h', 'i', 'j', 'k', 'l', 'm', 'n',
                             'o', 'p', 'q', 'r', 's', 't', 'u',
                             'v', 'w', 'x', 'y', 'z'};
    auto rand_char = [&alphabet]() -> char {
        int random = rand() % sizeof(alphabet);
        return alphabet[random];
    };

    std::string s(length, 0);
    std::generate_n(s.begin(), length, rand_char);
    return s;
}

void Test() {
    for (size_t i = 0; i < 1000; ++i) {
        SuffixAutomaton aut = SuffixAutomaton();
        std::string s = GenerateString(50);
        for (auto c : s) {
            aut.AddChar(c);
        }

        for (size_t j = 0; j < 100; ++j) {
            size_t l = rand() % s.size();
            size_t r = rand() % (s.size() - l);
            assert(aut.Find(s.substr(l, r)));
        }

        for (size_t j = 0; j < 100; ++j) {
            std::string check_s = GenerateString((rand() % 70));
            assert((s.find(check_s) != std::string::npos) == aut.Find(check_s));
        }
    }
}

std::string GetInput(std::istream& is) {
    size_t n, m;
    is >> n >> m;
    std::string s(n, 0);
    for (size_t i = 0; i < n; ++i) {
        int k;
        is >> k;
        s[i] = 'a' + k;
    }
    return s;
}

void GetOutput(ssize_t refren, const std::vector<char>& buffer, std::ostream& os) {
    os << refren << "\n";
    os << buffer.size() << "\n";
    for (auto it = buffer.rbegin(); it != buffer.rend(); ++it) {
        os << static_cast<int>(*it) - static_cast<int>('a') << " ";
    }
}

void Solve(std::istream& is, std::ostream& os) {
    SuffixAutomaton automaton = SuffixAutomaton();
    automaton.AddString(GetInput(is));
    RefrenFinder finder(automaton);
    auto refren(finder.GetRefren());
    GetOutput(refren.refrenValue, refren.refrenString, os);
}

int main() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);
    Solve(std::cin, std::cout);
    return 0;
}

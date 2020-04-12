#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <cassert>
#include <algorithm>
#include <map>
#include <memory>
#include <cmath>
#include <istream>

class SuffixTree {
private:
    struct Node;
public:
    typedef Node NodeType;

    explicit SuffixTree(const std::string& s) :
            lastNotLeaf_(Position(nullptr, -1)),
            sizeOfText_(s.size()),
            string_("")
    {
        parentRoot_ = std::make_shared<Node>(-1, -1, nullptr);
        root_ = std::make_shared<Node>(-1, 0, parentRoot_);
        root_->link = parentRoot_;
        lastNotLeaf_ = Position(root_, 0);
        addString(s);
    }

    ~SuffixTree() = default;

    void addString(const std::string& s) {
        for (auto i : s) {
            add(i);
        }
        findCounts(root_);
    }

    void add(char c) {
        string_.push_back(c);
        while (!canGo(lastNotLeaf_, c)) {
            std::shared_ptr<Node> node = makeNode(lastNotLeaf_);
            node->children[c] = std::make_shared<Node>(string_.size() - 1, sizeOfText_, node);
            lastNotLeaf_ = Position(getLink(node), 0);
        }
        lastNotLeaf_ = go(lastNotLeaf_, c);
    }

    std::shared_ptr<const Node> getRoot() const {
        return root_;
    }

    const std::string& getString() const {
        return string_;
    }

    ssize_t getSizeOfText() const {
        return sizeOfText_;
    }

private:

    struct Node {
        Node(ssize_t edgeStart_, ssize_t edgeFinish_, std::shared_ptr<Node> parent_) :
                edgeStart(edgeStart_),
                edgeFinish(edgeFinish_),
                parent(std::move(parent_)),
                link(nullptr),
                lengthOfSubstring(0),
                count(0) {}

        ~Node() = default;

        ssize_t getLength() const {
            return edgeFinish - edgeStart;
        }

        ssize_t getStartIndex() const {
            return edgeFinish - lengthOfSubstring;
        }


        std::map<char, std::shared_ptr<Node>> children;
        ssize_t edgeStart;
        ssize_t edgeFinish;
        std::shared_ptr<Node> link;
        std::shared_ptr<Node> parent;

        ssize_t lengthOfSubstring;
        ssize_t count;
    };

    struct Position {
        std::shared_ptr<Node> node;
        ssize_t dist;
        Position(std::shared_ptr<Node> node_, ssize_t dist_) : node(std::move(node_)), dist(dist_) {}
    };

    bool canGo(const Position& pos, char c) const {
        if (isNode(pos)) {
            return (pos.node == parentRoot_) || (pos.node->children.count(c) > 0);
        } else {
            return string_[pos.node->edgeFinish - pos.dist] == c;
        }
    }

    Position go(const Position& pos, char c) const {
        if (isNode(pos)) {
            return getNextPosition(pos.node, c);
        } else {
            return Position(pos.node, pos.dist - 1);
        }
    }

    Position getNextPosition(const std::shared_ptr<Node>& node, char c) const {
        std::shared_ptr<Node> child;
        if (node == parentRoot_) {
            child = root_;
        } else {
            child = node->children[c];
        }
        return Position(child, child->edgeFinish - child->edgeStart - 1);
    }

    bool isNode(const Position& pos) const {
        return pos.dist == 0;
    }

    std::shared_ptr<Node> makeNode(const Position& pos) const {
        if (isNode(pos)) {
            return pos.node;
        }
        std::shared_ptr<Node> childNode = pos.node;
        std::shared_ptr<Node> parentNode = childNode->parent;
        std::shared_ptr<Node> newNode = std::make_shared<Node>(childNode->edgeStart, childNode->edgeFinish - pos.dist, parentNode);
        childNode->edgeStart = childNode->edgeFinish - pos.dist;
        childNode->parent = newNode;
        parentNode->children[string_[newNode->edgeStart]] = newNode;
        newNode->children[string_[childNode->edgeStart]] = childNode;
        return newNode;
    }

    std::shared_ptr<Node> getLink(const std::shared_ptr<Node>& node) {
        if (!node->link) {
            node->link = buildLink(node);
        }
        return node->link;
    }

    std::shared_ptr<Node> buildLink(const std::shared_ptr<Node>& node) {
        std::shared_ptr<Node> parent = node->parent;
        Position pos(getLink(parent), 0);
        ssize_t l = node->edgeStart;
        ssize_t r = node->edgeFinish;
        while (l < r) {
            if (isNode(pos)) {
                pos = getNextPosition(pos.node, string_[l]);
                l++;
            }
            ssize_t len = std::min(r - l, pos.dist);
            pos.dist -= len;
            l += len;
        }
        return makeNode(pos);
    }

    void findCounts(const std::shared_ptr<Node>& node) {
        if (node->children.empty()) {
            node->count = 1;
            return;
        }
        for (auto i : node->children) {
            i.second->lengthOfSubstring = node->lengthOfSubstring + i.second->edgeFinish - i.second->edgeStart;
            findCounts(i.second);
            node->count += i.second->count;
        }
    }

    std::string string_;
    std::shared_ptr<Node> root_;
    std::shared_ptr<Node> parentRoot_;
    Position lastNotLeaf_;
    ssize_t sizeOfText_;
};

struct Answer{
    Answer(const std::string& refrenString_, ssize_t refrenValue_) :
        refrenString(refrenString_),
        refrenValue(refrenValue_)
    {}
    std::string refrenString;
    ssize_t refrenValue;
};

class RefrenFinder {
public:
    typedef SuffixTree::NodeType Node;
    RefrenFinder(const std::string& string_) :
            refrenValue(-1),
            refrenNode(nullptr),
            tree(string_ + '$')
    {}
    ~RefrenFinder() = default;

    Answer getRefren() {
        findRefren(tree.getRoot());
        if (refrenNode->edgeFinish == tree.getSizeOfText()) {
            refrenValue -= 1;
            return {tree.getString().substr(refrenNode->getStartIndex(), refrenNode->lengthOfSubstring - 1), refrenValue};
        }
        return {tree.getString().substr(refrenNode->getStartIndex(), refrenNode->lengthOfSubstring), refrenValue};
    }
private:

    void findRefren(const std::shared_ptr<const Node>& node) {
        for (auto i : node->children) {
            findRefren(i.second);
        }
        relaxRefren(node);
    }

    void relaxRefren(const std::shared_ptr<const Node>& node) {
        if (getValue(node) > refrenValue) {
            refrenValue = getValue(node);
            refrenNode = node;
        } else if (getValue(node) == refrenValue) {
            if (!refrenNode || node->lengthOfSubstring <= refrenNode->lengthOfSubstring) {
                refrenNode = node;
            }
        }
    }

    ssize_t getValue(const std::shared_ptr<const Node>& node) const {
        return node->count * node->lengthOfSubstring;
    }

    SuffixTree tree;
    std::shared_ptr<const Node> refrenNode;
    ssize_t refrenValue;
};


std::string get_input(std::istream& is) {
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

void get_output(const std::string& s, ssize_t value, std::ostream& os) {
    os << value << "\n";
    os << s.size() << "\n";
    for (auto i : s) {
        os << static_cast<int>(i) - static_cast<int>('a') << " ";
    }
}

void process(std::istream& is, std::ostream& os) {
    RefrenFinder finder(get_input(is));
    auto refren = finder.getRefren(); //std::move
    get_output(refren.refrenString, refren.refrenValue, os);
}


int main() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);
    process(std::cin, std::cout);
    return 0;
}


#include <iostream>
#include <vector>
#include <queue>
#include <string>
#include <algorithm>
#include <cstddef>

class Network {
private:
    class Edge;
    class EdgeIterrator;

    const std::size_t INF = 1000000000;

    std::size_t vertexCount_;
    std::size_t source_, target_;

    std::vector<Edge> edges_;
    std::vector<std::vector<std::size_t> > network_;
    std::vector<EdgeIterrator> vIter_;
    std::vector<std::size_t> distance_;

    std::size_t maxFlow_;

public:
    Network (std::size_t vertexCount, std::size_t source, std::size_t target) 
        : vertexCount_ (vertexCount), source_ (source), target_ (target)
        , maxFlow_ (0), network_ (vertexCount), distance_(vertexCount)
    {
        for (std::size_t i = 0; i < vertexCount_; ++i) {
            vIter_.push_back (begin(i));
        }
    }

    int addEdge (std::size_t from, std::size_t to, std::size_t capasity) 
    {
        std::size_t id = edges_.size();
        edges_.push_back (Edge (from, to, capasity));
        edges_.push_back (Edge (to, from, 0));

        network_[from].push_back (id);
        network_[to].push_back (id | 1);
        return id >> 1;
    }

    void addCapasity (std::size_t id, std::size_t addCapasity)
    {
        id <<= 1;
        edges_[id].addCapasity (addCapasity);
    }

    int getMaxFlow () 
    {
        return dinic();
    }

    EdgeIterrator begin(std::size_t v){
        return EdgeIterrator (network_[v].begin(), &edges_);
    }
    EdgeIterrator end (std::size_t v) {
        return EdgeIterrator (network_[v].end(), &edges_);
    }

private:
    class Edge {
    public:
        std::size_t capasity_, from_, to_;
        int flow_;
        Edge (std::size_t from, std::size_t to, std::size_t capasity) 
            : from_ (from), to_ (to), capasity_ (capasity), flow_ (0) 
        {}

        std::size_t getRest () {
            if (flow_ >= 0) {
                return capasity_ - (std::size_t) ( flow_);
            } else {
                return capasity_ + (std::size_t) (-flow_);
            }
            
        }
        bool isFull () {
            return getRest() == 0;
        }
        void addCapasity (int addCapasity)
        {
            capasity_ += addCapasity;
        }
    };

    class EdgeIterrator 
        : public std::iterator<std::random_access_iterator_tag,
                               Edge,
                               ptrdiff_t,
                               Edge*,
                               Edge&>
    {
    private:
        friend class Edge;

        std::vector<std::size_t>::iterator it_;
        std::vector<Edge> *pEdges_;
    public:

        EdgeIterrator (std::vector<std::size_t>::iterator it, std::vector<Edge> *pEdges)
        {
            it_ = it;
            pEdges_ = pEdges;
        }

        EdgeIterrator (const EdgeIterrator& edgeIterrator) = default;
        ~EdgeIterrator (){}

        EdgeIterrator& operator= (const EdgeIterrator& edgeIterator) = default;

        bool operator== (const EdgeIterrator& edgeIterator) const
        {
            return (it_ == edgeIterator.getConstPtr());
        }
        bool operator!= (const EdgeIterrator& edgeIterator) const 
        {
            return (it_ != edgeIterator.getConstPtr());
        }

        EdgeIterrator& operator+= (const ptrdiff_t& movement)
        {
            it_ += movement;
            return (*this);
        }
        EdgeIterrator& operator-= (const ptrdiff_t& movement)
        {
            it_ -= movement;
            return (*this);
        }
        EdgeIterrator& operator++ ()
        {
            ++it_;
            return (*this);
        }
        EdgeIterrator& operator-- ()
        {
            --it_;
            return (*this);
        }
        EdgeIterrator operator++ (int)
        {
            auto tmp(*this);
            ++it_;
            return tmp;
        }
        EdgeIterrator operator-- (int)
        {
            auto tmp(*this);
            --it_;
            return tmp;
        }
        EdgeIterrator operator+ (const ptrdiff_t& movement)
        {
            auto oldPtr = it_;
            it_ += movement;
            auto tmp(*this);
            it_ = oldPtr;
            return tmp;
        }
        EdgeIterrator operator- (const ptrdiff_t& movement)
        {
            auto oldPtr = it_;
            it_ -= movement;
            auto tmp(*this);
            it_ = oldPtr;
            return tmp;
        }

        ptrdiff_t operator- (const EdgeIterrator& edgeIterator)
        {
            return std::distance(edgeIterator.getPtr(),this->getPtr());
        }

        Edge& operator* () {
            return (*pEdges_)[*it_];
        }
        const Edge& operator* () const 
        {
            return (*pEdges_)[*it_];
        }
        Edge* operator-> () 
        {
            return &((*pEdges_)[*it_]);
        }
        Edge* backEdge () {
            return &((*pEdges_)[(*it_)^1]);
        }
    private:
        std::vector<std::size_t>::iterator getPtr() const
        {
            return it_;
        }
        const std::vector<std::size_t>::iterator getConstPtr() const
        {
            return it_;
        }
    };

private:
    bool dinicBfs ()
    {
        std::queue<std::size_t> queue;
        for (auto& d_i : distance_) {
            d_i = INF;
        }
        distance_[source_] = 0;
        queue.push (source_);
        while (!queue.empty() && distance_[target_] == INF) {
            int from = queue.front();
            queue.pop();

            for (auto e_i = begin(from); e_i != end(from); ++e_i) {
                if (distance_[e_i->to_] == INF && !e_i->isFull()) {
                    distance_[e_i->to_] = distance_[e_i->from_] + 1;
                    queue.push (e_i->to_);
                }
            }
        }
        return distance_[target_] != INF;
    } 

    std::size_t dinicDfs (std::size_t v, std::size_t flow)
    {
        if (flow == 0 || v == target_) return flow;
        for (; vIter_[v] != end(v); ++vIter_[v]) {
            auto e_i = vIter_[v];

            if (distance_[e_i->to_] == distance_[v] + 1) {

                std::size_t pushed = dinicDfs(e_i->to_, std::min(flow, e_i->getRest()));

                if (pushed > 0) {
                    e_i->flow_ += pushed;
                    e_i.backEdge()->flow_ -= pushed;

                    return pushed;
                }
            }
        }
        return 0;
    }

    std::size_t dinic ()
    {
        std::size_t maxFlow = 0;
        while (dinicBfs()) {
            
            for (std::size_t i = 0; i < vertexCount_; ++i) {
                vIter_[i] = begin(i);
            }
            while (std::size_t pushed = dinicDfs (source_, INF)) {
                maxFlow += pushed;
            }
        }
        return maxFlow;
    }
};

std::vector<std::vector<int> > loves;
std::vector<int> boys;
int n, m;

bool getAnswer() {
    Network network (n + m + 2, 0, 1);
    std::vector<bool> girlsWatch (m, false);
    int countWatch = 0;

    for (std::size_t i = 0; i < boys.size(); ++i) {
        for (std::size_t j = 0; j < loves[boys[i]].size(); ++j) {
            girlsWatch[loves[boys[i]][j]] = true;
        }
    }

    for (int i = 0; i < m; ++i) {
        if (girlsWatch[i]) {
            countWatch++;
            network.addEdge (n + 2 + i, 1, 1);
        }
    }
    std::size_t j = 0;
    for (std::size_t i = 0; i < boys.size(); ++i) {
        for (; j < boys[i]; ++j) {
            network.addEdge (0, j + 2, 1);

            for (std::size_t k = 0; k < loves[j].size(); ++k) {
                if (girlsWatch[loves[j][k]] == 1) {
                    network.addEdge (j + 2, loves[j][k] + n + 2, 1);
                }
            }
        }
        for (std::size_t k = 0; k < loves[boys[i]].size(); ++k) {
            girlsWatch[loves[boys[i]][k]] = 0;
        }
        ++j;
    }

    return countWatch == network.getMaxFlow();
}

int main()
{
    std::cin >> n >> m;
    loves.resize(n);
    for (int i = 0; i < n; ++i) {
        int cnt;
        std::cin >> cnt;
        for (int j = 0; j < cnt; ++j) {
            int a;
            std::cin >> a;
            a--;
            loves[i].push_back(a);
        }
    }

    int q;
    std::cin >> q;
    for (int i = 0; i < q; ++i) {
        int cnt;
        std::cin >> cnt;
        boys.resize(cnt);
        for (int j = 0; j < cnt; ++j) {
            std::cin >> boys[j];
            boys[j]--;
        }
        if (getAnswer()) {
            std::cout << "Yes\n";
        } else {
            std::cout << "No\n";
        }
    }

    return 0;
}
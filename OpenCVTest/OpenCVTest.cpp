#include <opencv2\opencv.hpp>
#include <iostream>
#include <fstream>
#include <chrono>

using namespace std::chrono;
using namespace std;
using namespace cv;

//Class representing RGB vector
class RGBVector
{
public:

    RGBVector(int bchanel, int rchanel, int gchanel)
    {
        values[0] = bchanel;
        values[1] = rchanel;
        values[2] = gchanel;
    }

    int getValue(int index) const
    {
        return values[index];
    }

    // Calculate delta GetShortestDistance according to doc
    int getDistance(const RGBVector& vector) const
    {
        int dist = abs(getValue(0) - vector.getValue(0)) + abs(getValue(1) - vector.getValue(1)) + abs(getValue(2) - vector.getValue(2));
        return dist;
    }

    // Operation overload needed for set 
    bool operator< (const RGBVector& right) const
    {
        return (values[0] != right.values[0]) ? (values[0] < right.values[0]) : ((values[1] != right.values[1]) ? (values[1] < right.values[1]) : (values[2] < right.values[2]));
    }

private:
    int values[3];
};

struct TDTreeNode
{
    TDTreeNode(const RGBVector& vector) : vectorData(vector), leftChild(nullptr), rightChild(nullptr) {}

    int get(int index) const
    {
        return vectorData.getValue(index);
    }

    int GetShortestDistance(const RGBVector& vector) const
    {
        return vectorData.getDistance(vector);
    }

    RGBVector vectorData;
    TDTreeNode* leftChild;
    TDTreeNode* rightChild;
};

struct NodeComparator
{
    NodeComparator(int index) : index(index) {}

    bool operator()(const TDTreeNode& n1, const TDTreeNode& n2) const
    {
        return n1.vectorData.getValue(index) < n2.vectorData.getValue(index);
    }

    int index;
};

// 3D tree
class TDTree
{
public:
    TDTree(const set<RGBVector>& colorSet) : nodesVector(colorSet.begin(), colorSet.end())
    {
        treeRoot = CreateTree(0, nodesVector.size(), 0);
    }

    inline bool IsEmpty() const { return nodesVector.empty(); }

    inline int GetVisitedNodes() const { return visitedNodes; }

    inline int GetShortestDistance() const { return shortestDistance; }

    const RGBVector& FindNearestColor(const RGBVector& pt);

private:
    TDTreeNode* treeRoot = nullptr;
    TDTreeNode* closesNeighbourCandidate = nullptr;
    int shortestDistance = 0;
    int visitedNodes = 0;
    vector<TDTreeNode> nodesVector;

    TDTreeNode* CreateTree(int begin, int end, int index);
    void FindNearestNeighbour(TDTreeNode* root, const RGBVector& point, int index);
};

TDTreeNode* TDTree::CreateTree(int begin, int end, int index)
{
    if (end <= begin)
        return nullptr;

    auto n = begin + (end - begin) / 2;
    auto i = nodesVector.begin();
    //Using nth_element instead of median to find object to split on.
    nth_element(i + begin, i + n, i + end, NodeComparator(index));
    index = (index + 1) % 3;
    nodesVector[n].leftChild = CreateTree(begin, n, index);
    nodesVector[n].rightChild = CreateTree(n + 1, end, index);
    return &nodesVector[n];
}

void TDTree::FindNearestNeighbour(TDTreeNode* root, const RGBVector& point, int index)
{
    if (root == nullptr)
        return;

    ++visitedNodes;
    int currentDistance = root->GetShortestDistance(point);
    if (closesNeighbourCandidate == nullptr || currentDistance < shortestDistance) 
    {
        shortestDistance = currentDistance;
        closesNeighbourCandidate = root;
    }

    //Exact color was found
    if (shortestDistance == 0)
        return;

    int dx = root->get(index) - point.getValue(index);
    index = (index + 1) % 3;
    FindNearestNeighbour(dx > 0 ? root->leftChild : root->rightChild, point, index);
    if (dx * dx >= shortestDistance)
        return;
    FindNearestNeighbour(dx > 0 ? root->rightChild : root->leftChild, point, index);
}

const RGBVector& TDTree::FindNearestColor(const RGBVector& pt)
{
    if (treeRoot == nullptr)
    {
        //Return invalid data as a signal that tree is empty
        return { -1, -1, -1 };
    }
    closesNeighbourCandidate = nullptr;
    visitedNodes = 0;
    shortestDistance = 0;
    FindNearestNeighbour(treeRoot, pt, 0);
    return closesNeighbourCandidate->vectorData;
}

string ChooseMenu()
{
    int choosePalette;

    cout << "Plesae choose palette:\n"
        << "1. 16 colors\n"
        << "2. 27 colors\n"
        << "3. 64 colors\n"
        << "4. 128 colors\n"
        << "5. 256 colors\n"
        << "Or press any button to continue with default image. \n";


    cin >> choosePalette;

    switch (choosePalette)
    {
    case 1:
        return "Palettes/16colors.png";
    case 2:
        return "Palettes/27colors.png";
    case 3:
        return "Palettes/64colors.png";
    case 4:
        return "Palettes/128colors.png";
    case 5:
        return "Palettes/256colors.png";
    default:
        return "obraz-A.jpg";
    }
}

int main()
{
    auto fullProgramDurationStart = high_resolution_clock::now();

    string chosenPalette = ChooseMenu();
    string paletteImagePath = samples::findFile(chosenPalette);
    Mat3b paletteImage = imread(paletteImagePath, IMREAD_COLOR);

    ofstream logFile;
    logFile.open("log.txt");

    //Gather palette of colours from paletteImage
    auto start = high_resolution_clock::now();

    set<RGBVector> paletteColors;
    for (int r = 0; r < paletteImage.rows; ++r)
    {
        for (int c = 0; c < paletteImage.cols; ++c)
        {
            Vec3b color = paletteImage(r, c);
            paletteColors.insert({ color[0], color[1], color[2] });
        }
    }
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);

    logFile << "Numbers of color of the palette image: " << paletteColors.size() << endl;
    logFile << "Time it took to gather colors from palette image (in milliseconds): " << duration.count() << endl;

    start = high_resolution_clock::now();

    TDTree tdtree(paletteColors);

    stop = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(stop - start);

    logFile << "Time it took to create KDTree (in milliseconds): " << duration.count() << endl;

    string canvasImagePath = samples::findFile("obraz-B.jpg");
    Mat3b canvasImage = imread(canvasImagePath, IMREAD_COLOR);
    

    //Used only to get the number of colors in canvas before conversion
    set<RGBVector> canvasColors;

    start = high_resolution_clock::now();
    for (int r = 0; r < canvasImage.rows; ++r)
    {
        for (int c = 0; c < canvasImage.cols; ++c)
        {
            Vec3b& color = canvasImage(r, c);
            canvasColors.insert({ color[0], color[1], color[2] });

            RGBVector exchangeColor = tdtree.FindNearestColor({ color[0], color[1], color[2] });
            
            //Additional possible log - log every pixel with it's RGB value and what it was exchanged to. It creates very large file, comented out by default.
            //logFile << "Exchanged pixed[ " << r << "][" << c << "], color {" << (int)color[0] << "," << (int)color[1] << "," << (int)color[2] << "} with color {" << exchangeColor.getValue(0) << "," << exchangeColor.getValue(1) << "," << exchangeColor.getValue(2) << "}." << " GetShortestDistance: " << tdtree.GetShortestDistance() << " GetVisitedNodes nodes: " << tdtree.GetVisitedNodes() << endl;
            
            color[0] = exchangeColor.getValue(0);
            color[1] = exchangeColor.getValue(1);
            color[2] = exchangeColor.getValue(2);
        }

    }

    logFile << "Numbers of color of the canvas image: " << canvasColors.size() << endl;

    stop = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(stop - start);

    logFile << "Time of conversion(in seconds): " << (duration.count() / 1000) % 60 << endl;

    imwrite("obraz-C.png", canvasImage);

    stop = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(stop - fullProgramDurationStart);
    logFile << "Full program duration(in seconds): " << (duration.count() / 1000) % 60 << endl;
    logFile.close();

    return 0;
}
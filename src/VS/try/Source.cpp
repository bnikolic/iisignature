#include<iostream>
#include<fstream>
#include<thread>
#include<iomanip>
#include<chrono>
#include<set>
#include"logsig.hpp"
#include "arbitrarySig.hpp"
using namespace std;

void interrupt() {}
void interrupt1() {
  while (1) {
    ifstream ifs("C:\\play\\foo1.txt");
    if (!ifs.is_open())
      return;
    ifs.close();
    //exit(0);
    std::cout << "interrupted" << std::endl;
    using namespace std::chrono_literals;
    this_thread::sleep_for(1s);
  }
}

void setupGlobal() {
	ifstream f("c:\\play\\iisignature\\iisignature_data\\bchLyndon20.dat");
	auto s = new std::string(istreambuf_iterator<char>(f), {});
	g_bchLyndon20_dat = s->c_str();
}

void trial() {
	setupGlobal();
	LogSigFunction lsf(LieBasis::Lyndon);
	WantedMethods wm;
	wm.m_expanded = wm.m_compiled_bch = wm.m_log_of_signature = wm.m_simple_bch = false;
	wm.m_compiled_bch = true;
	makeLogSigFunction(2, 2, lsf, wm, interrupt);
	vector<double> signature{ 3,3,3.0 };
	vector<double> displacement{ 1,17.0 };
	lsf.m_f->go(signature.data(), displacement.data());
}

void trySVD() {
	setupGlobal();
	LogSigFunction lsf(LieBasis::Lyndon);
	WantedMethods wm;
	wm.m_expanded =wm.m_compiled_bch= wm.m_log_of_signature = wm.m_simple_bch = false;
	wm.m_log_of_signature = true;
	makeLogSigFunction(103, 4, lsf, wm, interrupt1);
}

void __fastcall foo(double* a, const double* b, double* c);
typedef void(__fastcall *F)(double*, const double*, double*);

int main1() { 
	trySVD();
	//trial();
	double a = 3, b = 4, v = 4;
	foo(&a, &b, &v);
	F ff = foo;
	//cout << 3 << "\n";
  return 0;
}

//returns all the max^length members of the set {0 .. max-1}^length
//in lexicographic order
//as sequences in one long vector. The output has size length*max^length.
std::vector<int> getAllSequences(int max, int length) {
  std::vector<int> all(length * (int)std::pow(max, length));
  vector<int> buffer(length,0);
  auto outit = all.begin();
  while (1) {
    for (int i : buffer)
      *(outit++) = i;
    bool found = false;
    for (int offsetToChange = length - 1; offsetToChange >= 0; --offsetToChange) {
      if (buffer[offsetToChange] < max-1) {
        ++buffer[offsetToChange];
        for (int i = offsetToChange + 1; i < length; ++i)
          buffer[i] = 0;
        found = true;
        break;
      }
    }
    if (!found)
      return all;
  }
}

bool isSymmetric(const vector<vector<double>>& squareMatrix) {
  size_t n = squareMatrix.size();
  double maxdiff = 0;
  for (size_t i = 0; i < n; ++i) {
    for (size_t j = 0; j < i; ++j) {
      double diff = std::fabs(squareMatrix[j][i] - squareMatrix[i][j]);
      maxdiff = std::max(diff, maxdiff);
    }
  }
  std::cout << "biggest assymetry: "<< maxdiff << "\n";
  return false;
}

//if you have no indeterminates, c will be a number, so get it
double numberFromCoeff(const Coefficient& coeff) {
  if (coeff.m_details.empty())
    return 0;
  if (coeff.m_details.size() == 1 && coeff.m_details[0].first.empty())
    return coeff.m_details[0].second;
  throw std::runtime_error("a coeff which isn't just a number shouldn't be here");
}

//This function prints out the matrix from the mth level of tensor space to itself
//corresponding to the dynkin map.
//It is abundantly clear that this matrix is not symmetric
//and therefore the dynkin map is not an *orthogonal* projection (in the obvious basis).
void dynkinExperiment(const int d, const int m, bool p1, bool p2) {
  using namespace IISignature_algebra;
  WordPool w(LieBasis::Lyndon);
  auto list = makeListOfLyndonWords(w, d, m);
  vector<LyndonWord*> wds;
  for (auto& v : list)
    for (auto wd : v)
      wds.push_back(wd);
  vector<size_t> sigLevelSizes{ (size_t)d };
  for (int level = 2; level <= m; ++level)
    sigLevelSizes.push_back(d*sigLevelSizes.back());
  auto mappingMatrix = makeMappingMatrix(d, m, w, wds, sigLevelSizes);
  const auto& map = mappingMatrix[m - 1];
  vector<vector<double>> fullMatrix(sigLevelSizes.back());
  for (auto& v : fullMatrix)
    v.resize(sigLevelSizes.back());
  auto& letters = list[0];
  vector<int> allSeqs = getAllSequences(d, m);
  int nWords = allSeqs.size() / m;
  for (int i = 0; i < nWords; ++i)
  {
    int offset = i*m;
    LyndonWord* firstLetter = letters[allSeqs[offset]];
    auto poly = polynomialOfWord(firstLetter);
    auto poly2 = polynomialOfWord(letters[allSeqs[offset + m - 1]]);
    for (int j = 1; j < m; ++j) {
      auto n = productPolynomials(w, &*poly, &*polynomialOfWord(letters[allSeqs[offset + j]]), 1 + j);
      poly = move(n);
      auto n2 = productPolynomials(w, &*polynomialOfWord(letters[allSeqs[offset + m-1-j]]), &*poly2, 1 + j);
      poly2 = move(n2);
    }
    if (!p1)
      poly.reset();
    if (!p2)
      poly2.reset();
    if (poly)//poly)
      for (auto &l : poly->m_data)
        for (auto& ll : l) {
          double co = numberFromCoeff(ll.second);
          for (auto p : map.at(ll.first))
            fullMatrix[i][p.first] += co*p.second;
        }
    if (poly2)//poly2)
      for (auto &l : poly2->m_data)
        for (auto& ll : l) {
          double co = numberFromCoeff(ll.second);
          for (auto p : map.at(ll.first))
            fullMatrix[i][p.first] += co*p.second;
        }
  }
  int pos = 0;
  if(1)
  for (auto &v : fullMatrix) {
    for (auto dd : v)
      std::cout << std::setw(2)<< dd << " ";
    if (1)
      for (int i = 0; i < m; ++i)
        std::cout << allSeqs[pos++];
    std::cout << "\n";
  }
  isSymmetric(fullMatrix);
}

std::vector<Letter> indexToWord(size_t index, int d, int m) {
  std::vector<Letter> o;
  for (int i = 0; i < m; ++i) {
    Letter dig = index % d;
    index = index / d;
    o.push_back(dig);
  }
  std::reverse(o.begin(), o.end());
  return o;
}

void printAMappingMatrix() {
  vector<Letter> myletters{ 0,0,1,1,2,2 };
  //vector<Letter> myletters{ 0,0,1,1,2,2,2 };
  //vector<Letter> myletters{ 0,0,0,0,1,1,1,1 };
  //vector<Letter> myletters{ 0,0,0,1,1,1,2,2,2 };
  //vector<Letter> myletters{ 0,0,1,2 };
  //vector<Letter> myletters{ 0,1,2};
  if (!std::is_sorted(myletters.begin(), myletters.end()) || myletters.at(0)!=0)
    throw "myletters must be sorted";
  int m = (int)myletters.size();
  int d = 1 + *std::max_element(myletters.begin(), myletters.end());
  using namespace IISignature_algebra;
//  WordPool w(LieBasis::Lyndon);
  WordPool w(LieBasis::StandardHall);
  bool printBrackets = LieBasis::StandardHall == w.m_basis;
  auto list = makeListOfLyndonWords(w, d, m);
  vector<LyndonWord*> wds;
  for (auto& v : list)
    for (auto wd : v)
      wds.push_back(wd);
  vector<size_t> sigLevelSizes{ (size_t)d };
  for (int level = 2; level <= m; ++level)
    sigLevelSizes.push_back(d*sigLevelSizes.back());
  auto mappingMatrix = makeMappingMatrix(d, m, w, wds, sigLevelSizes);
  LyndonWordToIndex lyndonWordToIndex;
  LetterOrderToLW letterOrderToLW;
  analyseMappingMatrixLevel(mappingMatrix, m, letterOrderToLW, lyndonWordToIndex);
  auto v = lookupInFlatMap(letterOrderToLW, myletters);
  std::set<size_t> usedTensorIndicesS;
  for (auto wd : v) {
    for (auto& i : mappingMatrix.back().at(wd))
      usedTensorIndicesS.insert(i.first);
    //printLyndonWordDigits(*wd, std::cout);
    //std::cout << "\n";
  }
  //vector<size_t> usedTensorIndices(usedTensorIndicesS.begin(), usedTensorIndicesS.end());
  std::map<size_t, size_t> bigIdx2SmallIdx;
  {
    size_t idx = 0;
    for (auto i : usedTensorIndicesS)
      bigIdx2SmallIdx[i] = idx++;
  }
  std::ofstream output("foo.txt");
  vector<vector<Letter>> tensorLetters;
  for (auto i : usedTensorIndicesS)
    tensorLetters.push_back(indexToWord(i, d, m));
  size_t initialSpaces = 1 + m;
  if (printBrackets) {
    initialSpaces += 3 * (m - 1);//two brackets and a comma
  }
  for (int i = 0; i < m; ++i) {
    for (int j = 0; j < initialSpaces; ++j)
      output << " ";
    for (const auto& j : tensorLetters)
      output << (char)('1'+j[i]);
    output << "\n";
  }
  for (auto wd : v) {
    vector<float> out(usedTensorIndicesS.size());
    for (auto& i : mappingMatrix.back().at(wd))
      out[bigIdx2SmallIdx.at(i.first)] = i.second;
    if(!printBrackets)
      printLyndonWordDigits(*wd, output);
    else
      printLyndonWordBracketsDigits(*wd, output);
    output << " ";
    vector<Letter> wd_letters;
    wd->iterateOverLetters([&](Letter l) {wd_letters.push_back(l); });
    //std::reverse(wd_letters.begin(), wd_letters.end());
    size_t wd_idx = 0;
    for (Letter l : wd_letters) {
      wd_idx *= d;
      wd_idx += l;
    }
    int wd_small_idx = -1;
    if (w.m_basis==LieBasis::Lyndon)
      wd_small_idx = (int)bigIdx2SmallIdx.at(wd_idx);
      
    for (size_t j = 0; j != out.size(); ++j) {
      float i = out[j];
      float nice = i < 0 ? -i : i;
      nice = (nice <= 9) ? nice : 9;
      //output << " ";
      if (w.m_basis == LieBasis::Lyndon && wd_small_idx == j) {
        std::cout << i << "\n";
        output << (nice != 1 ? "*" : "#");
      }
      else
        output << nice;
    }
    output << "\n";
  }
}

int main() {
  printAMappingMatrix();
  //trial();
  //printListOfLyndonWords(2, 5);
  //ArbitrarySig::printArbitrarySig(3, 6);
  //dynkinExperiment(2, 4, 1, 0);
  //dynkinExperiment(2, 4, 0, 1);
  //dynkinExperiment(2, 4, 1, 1);
  //trySVD();
  //setupGlobal();
  //calcFla(2, 4, interrupt);
  return 0;
}
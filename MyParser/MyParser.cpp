// MyParser.cpp : Defines the entry point for the console application.
//

#include "polynomial_parser.h"
#include <string>
#include <functional>
#include <cassert>

namespace polynomial
{
//*** Addition
Monom& Monom::operator+=(Monom const& other)
{
    assert(this->exp == other.exp);
    this->coeff += other.coeff;
    return *this;
}

Monom operator+(Monom const& x, Monom const& y)
{
    return Monom(x) += y;
}

Polynomial& Polynomial::operator+=(Polynomial const& other)
{
    for(size_t jj = 0; jj < other.size(); ++jj) {
        this->push_back(other[jj]);
    }
    this->normalize(false);

    return *this;
}

Polynomial& operator+(Polynomial const& x, Polynomial const& y)
{
    return Polynomial(x) += y;
}

//*** Subtraction
Monom operator-(Monom const& other)
{
    return Monom(-other.coeff, other.exp);
}

Monom& Monom::operator-=(Monom const& other)
{
    assert(this->exp == other.exp);
    this->coeff -= other.coeff;
    return *this;
}

Monom operator-(Monom const& x, Monom const& y)
{
    return Monom(x) -= y;
}

Polynomial& Polynomial::operator-=(Polynomial const& other)
{
    for(size_t jj = 0; jj < other.size(); ++jj) {
        this->push_back(-other[jj]);
    }
    this->normalize(false);

    return *this;
}

Polynomial& operator-(Polynomial const& x, Polynomial const& y)
{
    return Polynomial(x) -= y;
}

//*** Multiplication
Monom& Monom::operator*=(Monom const& other)
{
    this->coeff *= other.coeff;
    this->exp += other.exp;
    return *this;
}

Monom operator*(Monom const& x, Monom const& y)
{
    return Monom(x) *= y;
}

Polynomial& Polynomial::operator*=(Polynomial const& other)
{
    Polynomial self(*this);
    this->clear();
    for(size_t ii = 0; ii < self.size(); ++ii) {
        for(size_t jj = 0; jj < other.size(); ++jj) {
            this->push_back(self[ii] * other[jj]);
		}
	}
    this->normalize(false);
    return *this;
}

Polynomial operator*(Polynomial const& x, Polynomial const& y)
{
    return Polynomial(x) *= y;
}

//*** Division
Monom& Monom::operator/=(Monom const& other)
{
    this->coeff /= other.coeff;
    this->exp -= other.exp;
    return *this;
}
Monom operator/(Monom const& x, Monom const& y)
{
    return Monom(x) /= y;
}

Polynomial& Polynomial::operator/=(Polynomial const& other)
{
    Polynomial q;
    Polynomial r(*this);
    while(r.size() && r[0].exp >= other[0].exp) {
        Monom tmp = r[0] / other[0];
        q.push_back(tmp);
        r -= Polynomial(tmp) * other;
	}
    q.normalize(false);
    *this = q;
    return *this;
}

Polynomial operator/(Polynomial const& x, Polynomial const& y)
{
    return Polynomial(x) /= y;
}

std::ostream& operator<< (std::ostream& ostr, Monom const& oMonom)
{
    // Handle coefficient
	if(oMonom.exp != 0 
	   && (oMonom.coeff == 1.0 || oMonom.coeff == -1.0)) {
       if(oMonom.coeff < 0.0) {
           ostr << "-";
	   }
	   else if(ostr.flags() & ostr.showpos) {
          ostr << "+";
	   }
	}
	else {
        ostr << oMonom.coeff;
	}
    // Handle exponent
    if(oMonom.exp == 1) {
        ostr << "x";
    }
    else if(oMonom.exp != 0) {
        ostr << "x^" << oMonom.exp;
    }

    return ostr;
}

std::ostream& operator<< (std::ostream& ostr, Polynomial const& polynom)
{
    ostr << std::noshowpos;
    for(size_t ii = 0; ii < polynom.size(); ++ii) {
        if(ii > 0) {
            ostr << std::showpos;
		}
        ostr << polynom[ii];
	}
    return ostr;
}

std::ostream& operator<< (std::ostream& ostr, std::vector<Polynomial> const& vecPolynom)
{
    char const* delim = "";
    for(size_t ii = 0; ii < vecPolynom.size(); ++ii) {
        ostr << delim << vecPolynom[ii];
        delim = ", ";
	}
    return ostr;
}

void Polynomial::normalize(bool bIncr)
{
    if(bIncr) {
        std::sort(this->begin(), this->end(), std::less<Monom>());
	}
	else {
        std::sort(this->begin(), this->end(), std::greater<Monom>());
	}
    size_t ii = 0;
    for(size_t kk = 1; kk < this->size(); ++kk) {
        if((*this)[kk].exp == (*this)[ii].exp) {
            (*this)[ii].coeff += (*this)[kk].coeff;
		}
		else if(kk - ii > 1) {
            (*this)[++ii] = (*this)[kk];
		}
		else {
            ++ii;
		}
	}
    this->resize(ii+1);
    for(size_t ii = 0; ii < this->size();) {
        if((*this)[ii].coeff == 0.0) {
            this->erase(this->begin()+ii);
		}
		else {
            ++ii;
		}
	}
}

void eat_ws(std::istream& istr)
{
    while(isspace(istr.get()))
        ;
    istr.unget();
}

int get_non_ws(std::istream& istr)
{
    eat_ws(istr);
    return istr.get();
}

int peek_non_ws(std::istream& istr)
{
    eat_ws(istr);
    int c = istr.get();
    istr.unget();
    return c;
}

void PolynomialParser::parse(std::istream& istr, Polynomial& polynom)
{
    this->p_istr = &istr;

    Monom oMonom;
    polynom.clear();
    std::string coeff;
    std::string exponent;
    int c;
    bool bDoContinue = true;

    while(bDoContinue) {
        c = get_non_ws(istr);
        char cc = (char)c;

        if(this->get_state() == eSTART) {
            if(c == '+' || c == '-' || isdigit(c) || c == '.') {
                this->unget_switch_to(ePRE_COEFF);
			}
			else if(c == 'x') {
                coeff="1";
                this->switch_to(ePRE_EXP);
			}
			else {
                if(c != EOF) {
                    //throw poly_exception("ERROR: invalid syntax in polynomial");
                    istr.unget();
                    return;
				}
			}
		} else if(this->get_state() == ePRE_COEFF) {
            coeff.append(1, c);
            this->switch_to(eCOEFF);
		} else if(this->get_state() == eCOEFF) {
            if(isdigit(c) || c == '.') {
                coeff.append(1, c);
			}
            else if(c == 'x') {
                if(coeff == std::string("+") || coeff == std::string("-")) {
                    coeff += "1";
				}
                this->switch_to(ePRE_EXP);
			}
			else {
                this->unget_switch_to(eEND);
			}
		} else if(this->get_state() == ePRE_EXP) {
            if(c == '^') {
                this->switch_to(eEXP);
            }
            else {
                exponent = "1";
                this->unget_switch_to(eEND);
            }
		} else if(this->get_state() == eEXP) {
            if(isdigit(c)) {
                exponent.append(1, c);
			}
			else {
                this->unget_switch_to(eEND);
			}
		} else if(this->get_state() == eEND) {
            double cc = 1.0;
			if(!coeff.empty()) {
				cc = atof(coeff.c_str());
                coeff.clear();
			}
            unsigned ee = 0;
			if(!exponent.empty()) {
				ee = atoi(exponent.c_str());
                exponent.clear();
			}
            polynom.push_back(Monom(cc, ee));
            if(c == EOF) {
                bDoContinue = false;
			}
            this->unget_switch_to(eSTART);
		}
	}
}

void PolyListParser::parse(std::istream& istr, std::vector<Polynomial>& vecPolynom)
{
    bool bDoContinue = true;
    vecPolynom.push_back(Polynomial());
    int c = 0;

    while(bDoContinue) {
        this->oParser.parse(istr, vecPolynom.back());
        c = get_non_ws(istr);
        char cc = (char)c;
        if( c == ',' ) {
            vecPolynom.push_back(Polynomial());
		}
		else {
            bDoContinue = false;
		}
	}
}

} // end of namespace polynomial

void test_polynomial_mult()
{
    using namespace polynomial;

    std::vector<Polynomial> vecPolynom;
    PolyListParser oParser;
    oParser.parse(std::cin, vecPolynom);
    assert(vecPolynom.size() == 2);
    Polynomial const& x = vecPolynom[0];
    Polynomial const& y = vecPolynom[1];
    std::cout << "(" << x << ") * (" << y << ") = "
		      << (x * y) << std::endl;
}

void test_polynomial_div()
{
    using namespace polynomial;

    std::vector<Polynomial> vecPolynom;
    PolyListParser oParser;
    oParser.parse(std::cin, vecPolynom);
    assert(vecPolynom.size() == 2);
    Polynomial const& x = vecPolynom[0];
    Polynomial const& y = vecPolynom[1];
    std::cout << "(" << x << ") / (" << y << ") = "
		      << (x / y) << std::endl;
}


std::vector<polynomial::Polynomial> test_polynomial()
{
    using namespace polynomial;

    std::vector<Polynomial> vecPolynom;
    PolyListParser oParser;
    oParser.parse(std::cin, vecPolynom);
    //std::cout << vecPolynom << std::endl;
    return vecPolynom;
}

int main(int argc, char* argv[])
{

    //Polynomial oPolynom;
    //oPolynom.push_back(Monom(1.0));
    //oPolynom.push_back(Monom(-2.0, 1));
    //oPolynom.push_back(Monom(3.0, 2));
	//PolynomialParser oParser;
 //   oParser.parse(std::cin, oPolynom);

 //   std::cout << oPolynom << std::endl;

    //std::cout << test_polynomial() << std::endl;
    //test_polynomial_mult();
    test_polynomial_div();

	return 0;
}


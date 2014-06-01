#if !defined(POLYNOMIAL_PARSER_H)
#define POLYNOMIAL_PARSER_H

#include <iostream>
#include <vector>
#include <algorithm>

namespace polynomial
{
using std::size_t;

//*** Monomial
struct Monom
{
    Monom()
		: coeff(1.0), exp(0) {}

    explicit Monom(double _coeff, unsigned _exp=0)
		: coeff(_coeff), exp(_exp) {}

    Monom& operator+=(Monom const& other);
    Monom& operator-=(Monom const& other);
    Monom& operator*=(Monom const& other);
    Monom& operator/=(Monom const& other);

    double   coeff;
    unsigned exp;
};

bool operator<(Monom const& x, Monom const& y) {
    return x.exp < y.exp;
}

bool operator>(Monom const& x, Monom const& y) {
    return x.exp > y.exp;
}

Monom operator*(Monom const& x, Monom const& y);

std::ostream& operator<< (std::ostream& ostr, Monom const& oMonom);

//*** Polynomial
struct Polynomial
	: public std::vector<Monom>
{
	Polynomial() {}

    Polynomial(Monom const& oMonom) {
        this->push_back(oMonom);
	}
    Polynomial& operator+=(Polynomial const& other);
    Polynomial& operator-=(Polynomial const& other);
    Polynomial& operator*=(Polynomial const& other);
    Polynomial& operator/=(Polynomial const& other);
    void normalize(bool bIncr=false);
};

Polynomial operator*(Polynomial const& x, Polynomial const& y);

struct poly_exception:
	std::runtime_error
{
    poly_exception(std::string const& msg)
		: std::runtime_error(msg) {}
};

std::ostream& operator<< (std::ostream& ostr, Polynomial const& polynom);

//*** Parser for Polynomial
struct PolynomialParser
{
	enum State_t {eSTART, ePRE_COEFF, eCOEFF, ePRE_EXP, eEXP, eEND};

    PolynomialParser()
		: eState(eSTART), p_istr(0L) {}

    void parse(std::istream& istr, Polynomial& polynom);

private:

    void switch_to(State_t newState) {
        this->eState = newState;
	}

    void unget_switch_to(State_t newState) {
        p_istr->unget();
        this->switch_to(newState);
	}

    State_t get_state() const {
        return this->eState;
	}

    State_t eState;
    std::istream* p_istr;
};

//*** Parser for Sequence of polynomial
struct PolyListParser
{
	void parse(std::istream& istr, std::vector<Polynomial>& vecPolynom);

private:
    PolynomialParser oParser;
};

std::ostream& operator<< (std::ostream& ostr, std::vector<Polynomial> const& vecPolynom);

} // end of namespace polynomial

#endif // POLYNOMIAL_PARSER_H
#pragma once
#include <ql/quantlib.hpp>
#include "MustCapFloor.h"

MustCapFloor::MustCapFloor(EfficiencyProduct::EfficiencyTypeProduct typeProd, string paths, EfficiencyProduct::EfficiencyModelProduct modelProd, TiXmlHandle hdldoc, string trId) : EfficiencyLibProduct(typeProd,paths, modelProd, hdldoc, trId){
	modelProduct = modelProd;
	//cVol = new CapFloorVolatility();
	//capletsVolatilies = (cVol.buildFlatCurveVol(cVol));
	
}
void MustCapFloor::setComponentsQuantLib(){
	this->setComponentsForCapFloor();
	//index
	MustCapFloor::myIndex = ComponentIndexMust.ConstructIndex(forwardingTermStructure);

	////FloatingLeg
	SwapFloatingLeg floating_leg;
	floating_leg.setComponent(MustCapFloor::myIndex, floatingLegObj.basisQ, floatingLegObj.freqQ);

	Rates = floating_leg.vectorAdaptFreq(fixedRateObj.matrixRate, startDateObj.dateQ, endDateObj.dateQ, floatingLegObj.freqQ);

	if (ComponentIndexMust.spreads.size()<1) //condition to check the Sread
	{
		MustCapFloor::floatingLeg = floating_leg.ConstructLeg(startDateObj.dateQ, endDateObj.dateQ, ComponentPrincipalMust.nominal);
	}
	else
	{
		MustCapFloor::floatingLeg = floating_leg.ConstructLeg(startDateObj.dateQ, endDateObj.dateQ, ComponentPrincipalMust.nominal, ComponentIndexMust.matrixSpread);
	}
}

void MustCapFloor::makeCapFloor(Handle<QuantLib::YieldTermStructure> forwardingTermStructure){
//	this->forwardingTermStructure.linkTo(forwardingTermStructure);
	this->setComponentsQuantLib();
	//MustCapFloor::capfloor =  CapFloor(QuantLib::CapFloor::Cap, floatingLeg, Rates);
	MustCapFloor::capfloor = boost::shared_ptr<CapFloor> (new Cap(floatingLeg, Rates));
	//return boost::shared_ptr<CapFloor> CapFloor(new Cap(floatingLeg, Rates));
}

Real MustCapFloor::price(){
	switch (modelProduct) {
	case EfficiencyProduct::EfficiencyModelProduct::EFFICIENCY_LMM:
	{
		//TO DO
		return 0;
	}
	break;
	case EfficiencyProduct::EfficiencyModelProduct::QUANTLIB_LMM:
	{
		boost::shared_ptr<AnalyticCapFloorEngine> LmEngine = setEngineQuantLibLMM();

		capfloor->setPricingEngine(LmEngine);
	return	capfloor->NPV();
		//return 0;
	}
	break;
	case EfficiencyProduct::EfficiencyModelProduct::QUANTLIB_BLACKFT:
	{
		//TO DO

		boost::shared_ptr<BlackCapFloorEngine> strippedVolEngine = setEngineQuantLibBalckFT();
		 capfloor->setPricingEngine(strippedVolEngine);
		return capfloor->NPV();
	}
	break;
	case EfficiencyProduct::EfficiencyModelProduct::QUANTLIB_HULLWHITE:
	{
		//TO DO
		return 0;
	}
	break;
	default://just to do something
	{
		return 0;
	}
	break;
	}
}

boost::shared_ptr<BlackCapFloorEngine> MustCapFloor::setEngineQuantLibBalckFT(){

	CurveData cData;
	// Fill out with some sample market data
	cData.sampleMktData(cData);

	// Build a curve linked to this market data
	boost::shared_ptr<YieldTermStructure> ocurve = (cData.buildCurve(cData));

	// Link the curve to the term structure
	this->forwardingTermStructure.linkTo(ocurve);
	Date today(6, October, 2014);
	Settings::instance().evaluationDate() = today;

	//Date todaysDate = Date(8, Apr, 2015);
	//capletsVolatilies = buildCurveOptionletAtmVol(cVol);
	//capletsVolatilies = buildOptionletSurfaceVol(cVol);
	//capletsVolatilies = buildOptionletCurveVol(cVol);

	cVol.termStructure.linkTo(ocurve);
	capletsVolatilies = (cVol.buildFlatCurveVol(cVol));

	//capVol->enableExtrapolation();
	return boost::shared_ptr<BlackCapFloorEngine>(new
		BlackCapFloorEngine(forwardingTermStructure, capletsVolatilies));
}
boost::shared_ptr<AnalyticCapFloorEngine> MustCapFloor::setEngineQuantLibLMM(){
	CurveData cData;
	// Fill out with some sample market data
	cData.sampleMktData(cData);
	//Date today(10, October, 2014);
	//Settings::instance().evaluationDate() = today;
	// Build a curve linked to this market data
	boost::shared_ptr<YieldTermStructure> ocurve = (cData.buildCurve(cData));

	// Link the curve to the term structure
//	boost::shared_ptr<IborIndex> index = boost::shared_ptr<IborIndex>(new Euribor6M(forwardingTermStructure));
	Date todaysDate =
		myIndex->fixingCalendar().adjust(Date(6, October, 2014));
	Settings::instance().evaluationDate() = todaysDate;

	this->forwardingTermStructure.linkTo(ocurve);

	

	const Size size = floatingLeg.size();
	boost::shared_ptr<LiborForwardModelProcess> process(
		new LiborForwardModelProcess(size, myIndex));

	//// set-up the model
	///*	const Real a = 0.02;
	//const Real b = 0.12;
	//const Real c = 0.1;
	//const Real d = 0.01;*/

	const Real a = 0.025;
	const Real b = 0.12;
	const Real c = 0.1;
	const Real d = 0.01;

	boost::shared_ptr<LmVolatilityModel> volaModel(
		new LmLinearExponentialVolatilityModel(process->fixingTimes(), a, b, c, d));

	boost::shared_ptr<LmCorrelationModel> corrModel(
		new LmLinearExponentialCorrelationModel(size, 0.1, 0.1));

	boost::shared_ptr<LiborForwardModel> liborModel(
		new LiborForwardModel(process, volaModel, corrModel));

return	boost::shared_ptr<AnalyticCapFloorEngine> (
		new AnalyticCapFloorEngine(liborModel, forwardingTermStructure));
}
void MustCapFloor::setEngineQuantLibHullWhite(){

}
//boost::shared_ptr< PricingEngine >  MustCapFloor::SetPricingEngine(string pricingEngineName, Handle<QuantLib::YieldTermStructure> discountingTermStructure, Handle<QuantLib::YieldTermStructure> forwardingTermStructure){
//	if (pricingEngineName == "blackfloor"){
//		vol
//		Volatility vol = 0.01;
//
//		CapFloorVolatility capfloorVol;
//		capfloorVol.termStructure.linkTo(forwardingTermStructure.currentLink());
//		Handle<OptionletVolatilityStructure> capletsVolatilies;
//
//		capletsVolatilies = buildFlatCurveVol(capfloorVol);
//		//capletsVolatilies = buildCurveOptionletAtmVol(capfloorVol);
//		//capletsVolatilies = buildOptionletSurfaceVol(capfloorVol);
//		//capletsVolatilies = buildOptionletCurveVol(cVol);
//
//		boost::shared_ptr<BlackCapFloorEngine> strippedVolEngine(new BlackCapFloorEngine(discountingTermStructure, capletsVolatilies));
//		boost::shared_ptr<BlackCapFloorEngine> strippedVolEngine(new BlackCapFloorEngine(discountingTermStructure, vol));
//		return strippedVolEngine;
//	}
//	else return 0;
//
//}
//
//double MustCapFloor::Price(Handle<QuantLib::YieldTermStructure> discountingTermStructure, Handle<QuantLib::YieldTermStructure> forwardingTermStructure, int i, string pricingEngineName)
//{
//
//	index
//	boost::shared_ptr<IborIndex> myIndex = index.ConstructIndex(forwardingTermStructure);
//
//	Leg
//	Leg floatingLeg;
//	SwapFloatingLeg* floating_leg = new SwapFloatingLeg(myIndex, floating_LegFlow.basisQ, floating_LegFlow.freqQ);
//	if (index.spreads.size()<1) //condition to check the Sread
//	{
//		floatingLeg = floating_leg->ConstructLeg(startDate.dateQ, endDate.dateQ, principal.nominal);
//	}
//	else
//	{
//		floatingLeg = floating_leg->ConstructLeg(startDate.dateQ, endDate.dateQ, principal.nominal, index.matrixSpread);
//	}
//
//
//	boost::shared_ptr<CapFloor> CapFloor(new Cap(floatingLeg, floating_leg->vectorAdaptFreq(fixed_Rate.matrixRate, startDate.dateQ, endDate.dateQ, floating_LegFlow.freqQ)));
//
//#pragma region"Test"
//	double temp = std::get<2>(fixed_Rate.matrixRate[0]);
//	double temp1 = std::get<2>(fixed_Rate.matrixRate[1]);
//	double temp2 = std::get<2>(fixed_Rate.matrixRate[2]);
//	double temp3 = std::get<2>(fixed_Rate.matrixRate[3]);
//
//	Date dtemp = std::get<1>(fixed_Rate.matrixRate[0]);
//	Date dtemp1 = std::get<1>(fixed_Rate.matrixRate[1]);
//	Date dtemp2 = std::get<1>(fixed_Rate.matrixRate[2]);
//	Date dtemp3 = std::get<1>(fixed_Rate.matrixRate[3]);
//
//	Date xdtemp = std::get<0>(fixed_Rate.matrixRate[0]);
//	Date xdtemp1 = std::get<0>(fixed_Rate.matrixRate[1]);
//	Date xdtemp2 = std::get<0>(fixed_Rate.matrixRate[2]);
//	Date xdtemp3 = std::get<0>(fixed_Rate.matrixRate[3]);
//#pragma endregion
//	vol
//	Volatility vol = 0.01;
//
//	CapFloorVolatility capfloorVol;
//	capfloorVol.termStructure.linkTo(forwardingTermStructure.currentLink());
//	Handle<OptionletVolatilityStructure> capletsVolatilies;
//
//	capletsVolatilies = buildFlatCurveVol(capfloorVol);
//	//capletsVolatilies = buildCurveOptionletAtmVol(capfloorVol);
//	//capletsVolatilies = buildOptionletSurfaceVol(capfloorVol);
//	//capletsVolatilies = buildOptionletCurveVol(cVol);
//
//	boost::shared_ptr<BlackCapFloorEngine> strippedVolEngine(new BlackCapFloorEngine(discountingTermStructure, capletsVolatilies));
//	boost::shared_ptr<BlackCapFloorEngine> strippedVolEngine(new BlackCapFloorEngine(discountingTermStructure, vol));
//	CapFloor->setPricingEngine(strippedVolEngine);
//
//	return CapFloor->NPV();
//}
//double MustCapFloor::PriceNada_Imane(){
//
//#pragma region Vetor of Dates
//	std::vector <Date > date;
//	date.push_back(Date(7, October, 2014));
//	date.push_back(Date(15, October, 2014)); date.push_back(Date(22, October, 2014)); date.push_back(Date(29, October, 2014));
//	date.push_back(Date(10, November, 2014)); date.push_back(Date(8, December, 2014)); date.push_back(Date(8, January, 2015));
//	date.push_back(Date(9, Feb, 2015)); date.push_back(Date(9, March, 2015)); date.push_back(Date(8, April, 2015));
//	date.push_back(Date(8, July, 2015));
//	date.push_back(Date(8, October, 2015));
//
//	date.push_back(Date(8, April, 2016)); date.push_back(Date(10, October, 2016)); date.push_back(Date(10, April, 2017));
//	date.push_back(Date(9, October, 2017)); date.push_back(Date(9, April, 2018)); date.push_back(Date(8, October, 2018));
//	date.push_back(Date(8, April, 2019)); date.push_back(Date(8, October, 2019)); date.push_back(Date(8, April, 2020));
//	date.push_back(Date(8, October, 2020));
//
//
//
//	date.push_back(Date(8, April, 2021)); date.push_back(Date(8, October, 2021)); date.push_back(Date(8, April, 2022));
//	date.push_back(Date(10, October, 2022)); date.push_back(Date(10, April, 2023)); /*date.push_back(Date(9, October, 2023));
//																					date.push_back(Date(8, April, 2024)); date.push_back(Date(8, October, 2024)); date.push_back(Date(8, April, 2025));
//																					date.push_back(Date(8, October, 2025));
//
//																					date.push_back(Date(8, April, 2026)); date.push_back(Date(8, October, 2026)); date.push_back(Date(8, April, 2027));
//																					date.push_back(Date(8, October, 2027)); date.push_back(Date(10, April, 2028)); date.push_back(Date(9, October, 2028));
//																					date.push_back(Date(9, April, 2029)); date.push_back(Date(8, October, 2029)); date.push_back(Date(8, April, 2030));
//																					date.push_back(Date(8, October, 2030));
//
//																					date.push_back(Date(8, April, 2031)); date.push_back(Date(8, October, 2031)); date.push_back(Date(8, April, 2032));
//																					date.push_back(Date(8, October, 2032)); date.push_back(Date(8, April, 2033)); date.push_back(Date(10, October, 2033));
//																					date.push_back(Date(10, April, 2034)); date.push_back(Date(9, October, 2034)); date.push_back(Date(9, April, 2035));
//																					date.push_back(Date(8, October, 2035));
//
//
//																					date.push_back(Date(8, April, 2036)); date.push_back(Date(8, October, 2036)); date.push_back(Date(8, April, 2037));
//																					date.push_back(Date(8, October, 2037)); date.push_back(Date(8, April, 2038)); date.push_back(Date(10, October, 2038));
//																					date.push_back(Date(8, April, 2039)); date.push_back(Date(10, October, 2039)); date.push_back(Date(9, April, 2040));
//																					date.push_back(Date(8, October, 2040));
//
//																					date.push_back(Date(8, April, 2041)); date.push_back(Date(8, October, 2041)); date.push_back(Date(8, April, 2042));
//																					date.push_back(Date(8, October, 2042)); date.push_back(Date(8, April, 2043)); date.push_back(Date(8, October, 2043));
//																					date.push_back(Date(8, April, 2044)); date.push_back(Date(10, October, 2044)); date.push_back(Date(10, April, 2045));
//																					date.push_back(Date(9, October, 2045));
//
//
//																					date.push_back(Date(9, April, 2046)); date.push_back(Date(8, October, 2046)); date.push_back(Date(8, April, 2047));
//																					date.push_back(Date(8, October, 2047)); date.push_back(Date(8, April, 2048)); date.push_back(Date(8, October, 2048));
//																					date.push_back(Date(8, April, 2049)); date.push_back(Date(8, October, 2049)); date.push_back(Date(8, April, 2050));
//																					date.push_back(Date(10, October, 2050));
//
//																					date.push_back(Date(10, April, 2051)); date.push_back(Date(9, October, 2051)); date.push_back(Date(8, April, 2052));
//																					date.push_back(Date(8, October, 2052)); date.push_back(Date(8, April, 2053)); date.push_back(Date(8, October, 2053));
//																					date.push_back(Date(8, April, 2054)); date.push_back(Date(8, October, 2054)); date.push_back(Date(8, April, 2055));
//																					date.push_back(Date(8, October, 2055));
//
//																					date.push_back(Date(10, April, 2056)); date.push_back(Date(9, October, 2056)); date.push_back(Date(9, April, 2057));
//																					date.push_back(Date(8, October, 2057));	date.push_back(Date(8, April, 2058)); date.push_back(Date(8, October, 2058));
//																					date.push_back(Date(8, April, 2059)); date.push_back(Date(8, October, 2059)); date.push_back(Date(8, April, 2060));
//																					date.push_back(Date(8, October, 2060));
//
//																					date.push_back(Date(8, April, 2061)); date.push_back(Date(10, October, 2061)); date.push_back(Date(10, April, 2062));
//																					date.push_back(Date(9, October, 2062)); date.push_back(Date(9, April, 2063));	date.push_back(Date(8, October, 2063));
//																					date.push_back(Date(8, April, 2064)); date.push_back(Date(8, October, 2064));
//																					*/
//#pragma endregion 
//	Date settlmentDate(8, October, 2014);
//	std::vector <Date > date2 = vectorDates(settlmentDate, endDate.dateQ, floating_LegFlow.freqQ);
//
//	std::vector<Rate>  Fwd = Calcul_Fwd(date2);
//	int n = date2.size();
//	std::vector<Rate> fwdvect(n);
//	for (int i = 0; i < n; i++){
//		fwdvect[i] = Fwd[i] / 100;
//	}
//	Matrix VOLMATR1 = matRebonato(0.02, 0.1, 0.1, 0.01, date2);
//	int NmbrOfSimulation = 1000;
//	float price = principal.nominal[0] * PricingsimulationCap(fwdvect, VOLMATR1, fixed_Rate.rates[0], date2, startDate.dateQ, endDate.dateQ, settlmentDate, 2000);
//
//
//
//#pragma region "Test"
//	double temps = principal.nominal[0];
//	double temp1 = fixed_Rate.rates[0];
//
//	Date tempDate = date2[0];
//	Date tempDate1 = date2[1];
//	Date tempDate2 = date2[2];
//	Date tempDate3 = date2[n - 2];
//	Date tempDate4 = date2[n - 1];
//#pragma endregion
//	return price;
//}


/***

	stats.h

	Functions for statistical tests (Pearson chi-squared test, e.g.)

	Copyright (c) 2022 Chris Street.

***/
#ifndef __STATS_H__
#define __STATS_H__

/* The gamma function (Spouge's algorithm.) */
extern double gammaf(double N);

/* Fast approximation (Stirling's rule) to the gamma function. */
extern double approx_gamma(double Z);

/* dof = degrees of freedom, cv = critical value (chi^2 test statistic)
   output = p-value (probability that the test statistic will equal or exceed cv) */
extern double chisqr_p_val(int dof, double cv, bool use_approx = true);

/***

	Perform a Pearson chi-squared test.

	Inputs:
		npoints		Number of data buckets
		observed	Observed frequency of each bucket
		expected	Expected frequency of each bucket
	Input outputs:
		test_stat_out	if non-NULL, the chi-squared test statistic will be filled in at this address
	Return value:
		The p-value of the test (i.e. likelihood that the observed frequencies are consistent
		with the expected frequencies)

***/
extern double pearson_chisq_test(u32 npoints, double* observed, double* expected, double* test_stat_out = nullptr);

/* Given a z-score (the number of standard deviations from the mean), give the cumulative normal probability to the left of
   (smaller than) that z value. */
extern double prob_zscore_left(double z);

/* Given a z-score (the number of standard deviations from the mean), give the cumulative normal probability to the right of
   (larger than) that z value. */
extern double prob_zscore_right(double z);

/* Given a z-score (the number of standard deviations from the mean), give the total normal probability within that many 
   standard deviations of the mean (below or above, as in a two-tail test.) */
extern double prob_zscore_within(double z);

/* Given a z-score (the number of standard deviations from the mean), give the total normal probability above that many 
   standard deviations of the mean (below or above, as in a two-tail test.) */
extern double prob_zscore_beyond(double z);

/* Calculate the standard variance (and mean, if desired) of a data set. */
extern double standard_variance(double* points, u32 npoints, double* mean_out = nullptr);
extern double standard_variance(const std::vector<double>& points, double* mean_out = nullptr);

/* Calculate the standard deviation (and mean, if desired) of a data set. */
extern double standard_deviation(double* points, u32 npoints, double* mean_out = nullptr);
extern double standard_deviation(const std::vector<double>& points, double* mean_out = nullptr);

#endif  // __STATS_H__
/* end stats.h */
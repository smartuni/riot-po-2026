#include "moving_freq_avg.h"

moving_freq_avg_t* moving_freq_avg_new(int domain_size) {
	moving_freq_avg_t* avg = (moving_freq_avg_t*)malloc(sizeof(moving_freq_avg_t));
	if (!avg)
		return NULL;

	avg->domain_size = domain_size;
	avg->frequency_domain = (element_t*)malloc(sizeof(element_t) * domain_size);
	if (!avg->frequency_domain) {
		free(avg);
		return NULL;
	}

	// Initialize all elements to zero
	for (int i = 0; i < domain_size; i++) {
		avg->frequency_domain[i].sum = 0;
		avg->frequency_domain[i].count = 0;
		avg->frequency_domain[i].average = 0;
	}
	return avg;
}

static void finalize_frequency_average(element_t* element) {
	if (element->count > 0) {
		// Integer division truncates toward zero
		element->average = element->sum / element->count;
	} else {
		element->average = 0;
	}
}

void moving_freq_avg_add_sample(moving_freq_avg_t* avg, int frequency, int magnitude) {
	if (!avg || frequency < 0 || frequency >= avg->domain_size) {
		return; // Invalid input
	}

	// Add the new sample to the current frequency
	avg->frequency_domain[frequency].sum += magnitude;
	avg->frequency_domain[frequency].count++;
}

void moving_freq_avg_finalize(moving_freq_avg_t* avg) {
	if (!avg)
		return;

	for (int i = 0; i < avg->domain_size; i++) {
		finalize_frequency_average(&avg->frequency_domain[i]);
	}
}

void moving_freq_avg_delete(moving_freq_avg_t* avg) {
	if (avg) {
		free(avg->frequency_domain);
		free(avg);
	}
}
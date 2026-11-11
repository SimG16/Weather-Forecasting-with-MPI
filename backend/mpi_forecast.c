#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gsl/gsl_statistics.h>
#include <mpi.h>

#define MAX_LINE 256
#define MAX_ENTRIES 1001
#define FORECAST_DAYS 7

typedef struct {
    char date[15];
    float max_temp;
    float avg_temp;
    float min_temp;
    float avg_humidity;
    int max_wind_speed;
    float avg_hourly_wind_speed;
    float avg_wind_speed;
    int min_wind_speed;
    int min_uv_forecast;
    int max_uv_forecast;
    int min_high_temp_forecast;
    int max_high_temp_forecast;
    int min_low_temp_forecast;
    int max_low_temp_forecast;
} WeatherData;

// ARIMA-like simple moving average predictor with trend adjustment
void arima_forecast(float *data, int size, float *forecast) {
    if (size < 2) {
        for (int i = 0; i < FORECAST_DAYS; i++) {
            forecast[i] = 0.0;  // Prevent NaN propagation
        }
        return;
    }

    double temp_data[size];
    for (int i = 0; i < size; i++) {
        temp_data[i] = (double) data[i];
    }
    
    double mean = gsl_stats_mean(temp_data, 1, size);
    double trend = (temp_data[size-1] - temp_data[size-2]) / FORECAST_DAYS;
    
    for (int i = 0; i < FORECAST_DAYS; i++) {
        forecast[i] = (float) (mean + (i * trend));
    }
}

// ARIMA Forecast for all parameters
void arima_forecast_all(WeatherData *data, int size, WeatherData *forecast) {
    if (size < 2) {
        memset(forecast, 0, FORECAST_DAYS * sizeof(WeatherData));
        return;
    }
    
    float max_temp[size], avg_temp[size], min_temp[size], avg_humidity[size], avg_wind_speed[size];
    for (int i = 0; i < size; i++) {
        max_temp[i] = data[i].max_temp;
        avg_temp[i] = data[i].avg_temp;
        min_temp[i] = data[i].min_temp;
        avg_humidity[i] = data[i].avg_humidity;
        avg_wind_speed[i] = data[i].avg_wind_speed;
    }
    
    arima_forecast(max_temp, size, (float *) &forecast[0].max_temp);
    arima_forecast(avg_temp, size, (float *) &forecast[0].avg_temp);
    arima_forecast(min_temp, size, (float *) &forecast[0].min_temp);
    arima_forecast(avg_humidity, size, (float *) &forecast[0].avg_humidity);
    arima_forecast(avg_wind_speed, size, (float *) &forecast[0].avg_wind_speed);
}

int main(int argc, char **argv) {
    int rank, num_procs;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    char line[MAX_LINE];
    WeatherData data[MAX_ENTRIES];
    int i = 0;

    if (rank == 0) {
        FILE *weatherData = fopen("weather_data.csv", "r");
        if (weatherData == NULL) {
            printf("Error: could not open file\n");
            MPI_Abort(MPI_COMM_WORLD, -1);
        }
        fgets(line, MAX_LINE, weatherData);

        while (i < MAX_ENTRIES && fgets(line, MAX_LINE, weatherData)) {
            sscanf(line, "%[^,],%f,%f,%f,%f,%d,%f,%f,%d,%d,%d,%d,%d,%d,%d",
                   data[i].date, &data[i].max_temp, &data[i].avg_temp, &data[i].min_temp,
                   &data[i].avg_humidity, &data[i].max_wind_speed, &data[i].avg_hourly_wind_speed,
                   &data[i].avg_wind_speed, &data[i].min_wind_speed, &data[i].min_uv_forecast,
                   &data[i].max_uv_forecast, &data[i].min_high_temp_forecast,
                   &data[i].max_high_temp_forecast, &data[i].min_low_temp_forecast,
                   &data[i].max_low_temp_forecast);
            i++;
        }
        fclose(weatherData);
    }
    
    MPI_Bcast(&i, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(data, i * sizeof(WeatherData), MPI_BYTE, 0, MPI_COMM_WORLD);

    WeatherData arima_predictions[FORECAST_DAYS];
    arima_forecast_all(data, i, arima_predictions);

    if (rank == 0) {
        printf("\n==== 7-Day Weather Forecast ===="
               "\n| Day | Max Temp | Avg Temp | Min Temp | Humidity | Wind Speed |\n");
        for (int d = 0; d < FORECAST_DAYS; d++) {
            printf("| %3d | %9.2f | %8.2f | %8.2f | %8.2f | %10.2f |\n", d + 1, arima_predictions[d].max_temp, arima_predictions[d].avg_temp, arima_predictions[d].min_temp, arima_predictions[d].avg_humidity, arima_predictions[d].avg_wind_speed);
        }
    }
    
    MPI_Finalize();
    return 0;
}
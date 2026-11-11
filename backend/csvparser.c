#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <microhttpd.h>
#include <mpi.h>

#define PORT 8080
#define MAX_LINE 256
#define MAX_ENTRIES 1001
#define FORECAST_DAYS 14

typedef struct {
    char date[15];
    float max_temp;
    float avg_temp;
    float min_temp;
    float avg_humidity;
    float avg_wind_speed;
    float max_uv_forecast;
    float avg_health_index;
} WeatherData;

#define MASTER_NODE 0
#define WORKER_NODE_1 1
#define WORKER_NODE_2 2
#define WORKER_NODE_3 3

// Function to calculate the mean of an array
float calculate_mean(float *data, int size) {
    if (size == 0) return 0.0;
    float sum = 0.0;
    for (int i = 0; i < size; i++) {
        sum += data[i];
    }
    return sum / size;
}

// Function to calculate trend using a rolling window
float calculate_trend(float *data, int size, int window) {
    if (size < window + 1) return 0.0;

    float sum_trend = 0.0;
    for (int i = size - window; i < size - 1; i++) {
        sum_trend += (data[i + 1] - data[i]);
    }
    return sum_trend / window;
}

// ARIMA-like forecast function
void arima_forecast(float *data, int size, float *forecast) {
    printf("Starting ARIMA forecast...\n");
    fflush(stdout);
    if (size < 2) {
        for (int i = 0; i < FORECAST_DAYS; i++) {
            forecast[i] = data[size - 1];
        }
        printf("ARIMA forecast complete with fallback\n");
        fflush(stdout);
        return;
    }

    float mean = calculate_mean(data, size);
    float trend = calculate_trend(data, size, 5) / FORECAST_DAYS;

    for (int i = 0; i < FORECAST_DAYS; i++) {
        forecast[i] = mean + (i * trend);
    }
    printf("ARIMA forecast complete\n");
    fflush(stdout);
}

// Forecast all relevant weather parameters
void arima_forecast_all(WeatherData *data, int size, WeatherData *forecast) {
    printf("Starting ARIMA forecast for all parameters...\n");
    fflush(stdout);
    if (size < 2) {
        memset(forecast, 0, FORECAST_DAYS * sizeof(WeatherData));
        printf("ARIMA forecast skipped due to insufficient data\n");
        fflush(stdout);
        return;
    }

    float max_temp[size], avg_temp[size], min_temp[size], avg_humidity[size], avg_wind_speed[size], max_uv_forecast[size], avg_health_index[size];

    for (int i = 0; i < size; i++) {
        max_temp[i] = data[i].max_temp;
        avg_temp[i] = data[i].avg_temp;
        min_temp[i] = data[i].min_temp;
        avg_humidity[i] = data[i].avg_humidity;
        avg_wind_speed[i] = data[i].avg_wind_speed;
        max_uv_forecast[i] = data[i].max_uv_forecast;
        avg_health_index[i] = data[i].avg_health_index;
    }

    float temp_forecast[FORECAST_DAYS];

    arima_forecast(max_temp, size, temp_forecast);
    for (int i = 0; i < FORECAST_DAYS; i++) forecast[i].max_temp = temp_forecast[i];

    arima_forecast(avg_temp, size, temp_forecast);
    for (int i = 0; i < FORECAST_DAYS; i++) forecast[i].avg_temp = temp_forecast[i];

    arima_forecast(min_temp, size, temp_forecast);
    for (int i = 0; i < FORECAST_DAYS; i++) forecast[i].min_temp = temp_forecast[i];

    arima_forecast(avg_humidity, size, temp_forecast);
    for (int i = 0; i < FORECAST_DAYS; i++) forecast[i].avg_humidity = temp_forecast[i];

    arima_forecast(avg_wind_speed, size, temp_forecast);
    for (int i = 0; i < FORECAST_DAYS; i++) forecast[i].avg_wind_speed = temp_forecast[i];

    arima_forecast(max_uv_forecast, size, temp_forecast);
    for (int i = 0; i < FORECAST_DAYS; i++) forecast[i].max_uv_forecast = temp_forecast[i];

    arima_forecast(avg_health_index, size, temp_forecast);
    for (int i = 0; i < FORECAST_DAYS; i++) forecast[i].avg_health_index = temp_forecast[i];

    printf("ARIMA forecast for all parameters complete\n");
    fflush(stdout);
}

// Save the forecast data to a JSON file
void save_forecast_to_file(WeatherData *forecast) {
    printf("Saving forecast to file...\n");
    fflush(stdout);
    FILE *file = fopen("forecast.json", "w");
    if (file == NULL) {
        printf("Error: Unable to create forecast file.\n");
        fflush(stdout);
        return;
    }

    fprintf(file, "[\n");
    for (int i = 0; i < FORECAST_DAYS; i++) {
        fprintf(file,
            "  {\n"
            "    \"day\": %d,\n"
            "    \"max_temp\": %.2f,\n"
            "    \"avg_temp\": %.2f,\n"
            "    \"min_temp\": %.2f,\n"
            "    \"avg_humidity\": %.2f,\n"
            "    \"avg_wind_speed\": %.2f,\n"
            "    \"max_uv_forecast\": %.2f,\n"
            "    \"avg_health_index\": %.2f\n"
            "  }%s\n",
            i + 1,
            forecast[i].max_temp,
            forecast[i].avg_temp,
            forecast[i].min_temp,
            forecast[i].avg_humidity,
            forecast[i].avg_wind_speed,
            forecast[i].max_uv_forecast,
            forecast[i].avg_health_index,
            (i < FORECAST_DAYS - 1) ? "," : ""
        );
    }
    fprintf(file, "]\n");

    fclose(file);
    printf("Forecast saved to forecast.json\n");
    fflush(stdout);
}

struct MHD_Response *create_response_with_json(const char *json_str) {
    printf("Creating response with JSON...\n");
    fflush(stdout);
    struct MHD_Response *response;

    response = MHD_create_response_from_buffer(strlen(json_str),
                                               (void *)json_str,
                                               MHD_RESPMEM_MUST_COPY);
    if (!response) {
        fprintf(stderr, "Failed to create response buffer\n");
        fflush(stdout);
        return NULL;
    }

    MHD_add_response_header(response, "Content-Type", "application/json");
    return response;
}

void handle_request(int rank, const char *region) {
    printf("Handling request for region: %s\n", region);
    fflush(stdout);
    char filename[100];
    if (strcmp(region, "Peel") == 0) {
        strcpy(filename, "Peel.csv");
    } else if (strcmp(region, "York") == 0) {
        strcpy(filename, "York.csv");
    } else if (strcmp(region, "Niagara") == 0) {
        strcpy(filename, "Niagara.csv");
    } else {
        printf("Unknown region\n");
        fflush(stdout);
        return;
    }

    FILE *weatherData = fopen(filename, "r");
    if (!weatherData) {
        printf("Unable to open CSV file\n");
        fflush(stdout);
        return;
    }

    WeatherData data[MAX_ENTRIES];
    int i = 0;
    char line[MAX_LINE];
    while (fgets(line, MAX_LINE, weatherData) && i < MAX_ENTRIES) {
        sscanf(line, "%14[^,],%f,%f,%f,%f,%f,%f,%f",
               data[i].date, &data[i].max_temp, &data[i].avg_temp,
               &data[i].min_temp, &data[i].avg_humidity, &data[i].avg_wind_speed, &data[i].max_uv_forecast, &data[i].avg_health_index);
        i++;
    }
    fclose(weatherData);

    WeatherData forecast[FORECAST_DAYS];
    arima_forecast_all(data, i, forecast);

    // Send forecast data to master
    MPI_Send(forecast, FORECAST_DAYS * sizeof(WeatherData), MPI_BYTE, MASTER_NODE, 0, MPI_COMM_WORLD);
}

void master_node_handler(struct MHD_Connection *connection, const char *region) {
    printf("Master node handling request for region: %s\n", region);
    fflush(stdout);

    WeatherData forecast[FORECAST_DAYS];

    // Send task to workers
    int worker_rank = (strcmp(region, "Peel") == 0) ? WORKER_NODE_1 :
                      (strcmp(region, "York") == 0) ? WORKER_NODE_2 :
                      (strcmp(region, "Niagara") == 0) ? WORKER_NODE_3 : -1;

    if (worker_rank == -1) {
        printf("Invalid region: %s\n", region);
        fflush(stdout);
        const char *error_json = "{\"error\": \"Invalid region specified\"}";
        struct MHD_Response *response = create_response_with_json(error_json);
        MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, response);
        MHD_destroy_response(response);
        return;
    }

    MPI_Send(region, 50, MPI_CHAR, worker_rank, 0, MPI_COMM_WORLD);
    printf("Sent request to worker %d\n", worker_rank);
    fflush(stdout);

    // Receive data from the worker node
    MPI_Recv(forecast, FORECAST_DAYS * sizeof(WeatherData), MPI_BYTE, worker_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    printf("Received forecast from worker %d\n", worker_rank);
    fflush(stdout);

    // Save forecast to JSON file
    save_forecast_to_file(forecast);

    // Read the saved forecast from "forecast.json"
    FILE *file = fopen("forecast.json", "r");
    if (!file) {
        printf("Error: Unable to open forecast.json\n");
        fflush(stdout);
        const char *error_json = "{\"error\": \"Unable to retrieve forecast data\"}";
        struct MHD_Response *response = create_response_with_json(error_json);
        MHD_queue_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
        MHD_destroy_response(response);
        return;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    // Allocate memory to read file
    char *json_data = (char *)malloc(file_size + 1);
    if (!json_data) {
        printf("Error: Memory allocation failed\n");
        fflush(stdout);
        fclose(file);
        const char *error_json = "{\"error\": \"Memory allocation failed\"}";
        struct MHD_Response *response = create_response_with_json(error_json);
        MHD_queue_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
        MHD_destroy_response(response);
        return;
    }

    // Read file into memory
    fread(json_data, 1, file_size, file);
    json_data[file_size] = '\0';  // Null-terminate the JSON string
    fclose(file);

    // Send the actual forecast JSON data
    struct MHD_Response *response = create_response_with_json(json_data);
    MHD_add_response_header(response, "Access-Control-Allow-Origin", "*");
    MHD_add_response_header(response, "Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    MHD_add_response_header(response, "Access-Control-Allow-Headers", "Content-Type");
    MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);

    free(json_data);
}

enum MHD_Result request_handler(void *cls, struct MHD_Connection *connection,
    const char *url, const char *method,
    const char *version, const char *upload_data,
    size_t *upload_data_size, void **con_cls)
{
    const char *region = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "region");
    printf("Request handler called, region: %s\n", region);
    fflush(stdout);

    if (!region) {
        const char *error_json = "{\"error\": \"Region parameter missing\"}";
        struct MHD_Response *response = create_response_with_json(error_json);
        MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, response);
        MHD_destroy_response(response);
        return MHD_HTTP_BAD_REQUEST;
    }

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == MASTER_NODE) {
        master_node_handler(connection, region);
    } else {
        handle_request(rank, region);
    }

    return MHD_HTTP_OK;
}

int main(int argc, char *argv[]) {
    int rank;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    struct MHD_Daemon *daemon = NULL;

    if (rank == MASTER_NODE) {
        daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL,
                                  &request_handler, NULL, MHD_OPTION_END);
        if (daemon == NULL) {
            printf("Error starting HTTP daemon.\n");
            MPI_Finalize();
            return 1;
        }
        printf("Server running on port %d\n", PORT);
        fflush(stdout);

        getchar();  // Keep server running
        MHD_stop_daemon(daemon);
    } else {
        printf("Worker node %d initialized\n", rank);
        fflush(stdout);

        // Workers listen for tasks indefinitely
        while (1) {
            char region[50];
            MPI_Status status;

            // Receive region name from master
            MPI_Recv(region, 50, MPI_CHAR, MASTER_NODE, 0, MPI_COMM_WORLD, &status);
            if (strcmp(region, "exit") == 0) break; // Stop worker when "exit" is received

            handle_request(rank, region);
        }
    }

    MPI_Finalize();
    return 0;
}
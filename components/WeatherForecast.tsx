import React, { JSX, useState, useEffect } from "react"; 
import "./WeatherForecast.css";

import { TiWeatherCloudy } from "react-icons/ti";
import { TiWeatherDownpour } from "react-icons/ti";
import { TiWeatherNight } from "react-icons/ti";
import { TiWeatherPartlySunny } from "react-icons/ti";
import { TiWeatherShower } from "react-icons/ti";
import { TiWeatherSnow } from "react-icons/ti";
import { TiWeatherStormy } from "react-icons/ti";
import { TiWeatherSunny } from "react-icons/ti";
import { TiWeatherWindy } from "react-icons/ti";
import { TiWeatherWindyCloudy } from "react-icons/ti";

const regions = ["Peel", "York", "Niagara"];

const stateIcons: Record<string, JSX.Element> = {
  cloudy: <TiWeatherCloudy size={30} color="#00000" />,
  downpour: <TiWeatherDownpour size={30} color="#00000" />,
  night: <TiWeatherNight size={30} color="#00000" />,
  partsunny: <TiWeatherPartlySunny size={30} color="#00000" />,
  rain: <TiWeatherShower size={30} color="#00000" />,
  snow: <TiWeatherSnow size={30} color="#00000" />,
  storm: <TiWeatherStormy size={30} color="#00000" />,
  sunny: <TiWeatherSunny size={30} color="#00000" />,
  windy: <TiWeatherWindy size={30} color="#00000" />,
  windycloudy: <TiWeatherWindyCloudy size={30} color="#00000" />,
};

const WeatherForecast = () => {
  const [selectedRegion, setSelectedRegion] = useState("Peel");
  const [forecast, setForecast] = useState<any[]>([]);
  const [isLoading, setIsLoading] = useState<boolean>(true);
  const [errorMessage, setErrorMessage] = useState<string | null>(null);

  useEffect(() => {
    const regionEncoded = encodeURIComponent(selectedRegion);
    const fetchUrl = `http://localhost:8080/forecast?region=${regionEncoded}`;
    console.log(`Fetching forecast for region: ${fetchUrl}`);

    setIsLoading(true);
    setErrorMessage(null);

    fetch(fetchUrl)
      .then((response) => {
        if (!response.ok) {
          throw new Error(`HTTP error! status: ${response.status}`);
        }
        return response.json();
      })
      .then((data) => {
        console.log("Fetched forecast data:", data);
        setForecast(data);
        setIsLoading(false);
      })
      .catch((error) => {
        console.error("Error fetching forecast data:", error);
        setErrorMessage("Failed to fetch forecast data. Please try again later.");
        setIsLoading(false);
      });
  }, [selectedRegion]);

  const handleRegionChange = (e: React.ChangeEvent<HTMLSelectElement>) => {
    const region = e.target.value;
    setSelectedRegion(region);
  };

  const getWeatherIcon = (condition: string) => {
    switch (condition) {
      case "cloudy":
        return stateIcons.cloudy;
      case "downpour":
        return stateIcons.downpour;
      case "night":
        return stateIcons.night;
      case "partsunny":
        return stateIcons.partsunny;
      case "rain":
        return stateIcons.rain;
      case "snow":
        return stateIcons.snow;
      case "storm":
        return stateIcons.storm;
      case "sunny":
        return stateIcons.sunny;
      case "windy":
        return stateIcons.windy;
      case "windycloudy":
        return stateIcons.windycloudy;
      default:
        return stateIcons.sunny;
    }
  };

  const firstHalf = forecast.slice(0, 7); // Days 1-7
  const secondHalf = forecast.slice(7, 14); // Days 8-14

  return (
    <div className="p-6 max-w-md mx-auto">
      <h1 className="text-2xl font-bold mb-4 text-center">Weather Forecast</h1>

      <label htmlFor="regions">Select a Region: </label>
      <select
        id="regions"
        name="regions"
        value={selectedRegion}
        onChange={handleRegionChange}
        className="mb-4"
      >
        {regions.map((region) => (
          <option key={region} value={region}>
            {region}
          </option>
        ))}
      </select>

      {isLoading ? (
        <p className="text-center">Loading forecast data...</p>
      ) : errorMessage ? (
        <p className="text-center text-red-500">{errorMessage}</p>
      ) : (
        forecast.length > 0 && (
          <>
            {/* Days 1-7 */}
            <div className="mt-4 p-4 text-center border rounded">
              <h2 className="text-lg font-semibold">{selectedRegion} - Days 1-7 Forecast</h2>
              <table className="table-auto w-full">
                <thead>
                  <tr>
                    {firstHalf.map((day, index) => (
                      <th key={index} className="p-2">Day {index + 1}</th>
                    ))}
                  </tr>
                </thead>
                <tbody>
                  <tr>
                    {firstHalf.map((day, index) => (
                      <td key={index} className="p-2">
                        <p className="font-bold">{day.avg_temp}°C</p>
                        <p>Min: {day.min_temp}°C</p>
                        <p>Max: {day.max_temp}°C</p>
                        <p>Humidity: {day.avg_humidity}%</p>
                        <p>Wind Speed: {day.avg_wind_speed} km/h</p>
                        <p>UV Index: {day.max_uv_forecast}</p>
                        <p>Health Index: {day.avg_health_index}</p>
                        {getWeatherIcon(day.condition)}
                      </td>
                    ))}
                  </tr>
                </tbody>
              </table>
            </div>

            {/* Days 8-14 */}
            <div className="mt-4 p-4 text-center border rounded">
              <h2 className="text-lg font-semibold">{selectedRegion} - Days 8-14 Forecast</h2>
              <table className="table-auto w-full">
                <thead>
                  <tr>
                    {secondHalf.map((day, index) => (
                      <th key={index} className="p-2">Day {index + 8}</th>
                    ))}
                  </tr>
                </thead>
                <tbody>
                  <tr>
                    {secondHalf.map((day, index) => (
                      <td key={index} className="p-2">
                        <p className="font-bold">{day.avg_temp}°C</p>
                        <p>Min: {day.min_temp}°C</p>
                        <p>Max: {day.max_temp}°C</p>
                        <p>Humidity: {day.avg_humidity}%</p>
                        <p>Wind Speed: {day.avg_wind_speed} km/h</p>
                        <p>UV Index: {day.max_uv_forecast}</p>
                        <p>Health Index: {day.avg_health_index}</p>
                        {getWeatherIcon(day.condition)}
                      </td>
                    ))}
                  </tr>
                </tbody>
              </table>
            </div>
          </>
        )
      )}
    </div>
  );
};

export default WeatherForecast;

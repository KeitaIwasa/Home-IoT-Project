// データのフェッチ
fetch('/data/last24hours')
  .then(response => response.json())
  .then(data => {
    const labels = data.map(point => new Date(point.timestamp).toLocaleTimeString());
    const tempData = data.map(point => point.temperature);
    const humidityData = data.map(point => point.humidity);

    // 温度データのグラフ
    const tempCtx = document.getElementById('tempChart').getContext('2d');
    const tempChart = new Chart(tempCtx, {
      type: 'line',
      data: {
        labels: labels,
        datasets: [{
          label: 'Temperature',
          data: tempData,
          fill: false,
          borderColor: 'red',
          tension: 0.1
        }]
      }
    });

    // 湿度データのグラフ
    const humidityCtx = document.getElementById('humidityChart').getContext('2d');
    const humidityChart = new Chart(humidityCtx, {
      type: 'line',
      data: {
        labels: labels,
        datasets: [{
          label: 'Humidity',
          data: humidityData,
          fill: false,
          borderColor: 'blue',
          tension: 0.1
        }]
      }
    });
  });

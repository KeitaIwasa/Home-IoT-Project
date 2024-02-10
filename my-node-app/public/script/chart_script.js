const ctx = document.getElementById('tempHumidityChart').getContext('2d');

// APIからデータを取得する関数
async function fetchData() {
    const response = await fetch('/data');
    const data = await response.json();
    return data;
}

// グラフを描画する関数
async function drawChart() {
    const data = await fetchData();

    const labels = data.map(entry => new Date(entry.timestamp).toLocaleTimeString());
    const tempData = data.map(entry => entry.temperature);
    const humidityData = data.map(entry => entry.humidity);

    const chart = new Chart(ctx, {
        type: 'line', // グラフのタイプ
        data: {
            labels: labels,
            datasets: [{
                label: 'Temperature (°C)',
                backgroundColor: 'rgba(255, 99, 132, 0.2)',
                borderColor: 'rgba(255, 99, 132, 1)',
                yAxisID: 'y',
                data: tempData,
            }, {
                label: 'Humidity (%)',
                backgroundColor: 'rgba(54, 162, 235, 0.2)',
                borderColor: 'rgba(54, 162, 235, 1)',
                yAxisID: 'y1',
                data: humidityData,
            }]
        },
        options: {
            scales: {
                y: {
                    type: 'linear',
                    display: true,
                    position: 'left',
                },
                y1: {
                    type: 'linear',
                    display: true,
                    position: 'right',
                    grid: {
                        drawOnChartArea: false,
                    },
                }
            }
        }
    });
}

drawChart();


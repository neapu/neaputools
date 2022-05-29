function test() {
    axios({
        method: 'get',
        url: '/api/get_data',
    }).then(function (response) {
        document.getElementById("show_data").innerText = response.data.data;
    });
}

document.getElementById("get_data").addEventListener("click", test);
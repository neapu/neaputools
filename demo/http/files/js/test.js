function test() {
    axios({
        method: 'get',
        url: '/api/get_data',
    }).then(function (response) {
        document.getElementById("show_data").innerText = response.data.data;
    });
}

function setCookie() {
    axios({
        method: 'get',
        url: '/api/set_cookie',
    }).then(function (response) {
        console.log(response.data.data);
    });
}

function getCookie() {
    axios({
        method: 'get',
        url: '/api/get_cookie',
    }).then(function (response) {
        console.log(response.data);
    });
}

document.getElementById("get_data").addEventListener("click", test);
document.getElementById("set_cookid").addEventListener("click", setCookie);
document.getElementById("get_cookie").addEventListener("click", getCookie);
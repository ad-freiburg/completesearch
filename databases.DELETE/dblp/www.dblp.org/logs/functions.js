var http_request = false;

function load_data() {
var content = "<? read_file('/var/log/dblp/dblp.log', 30); ?>";
document.getElementById("content").innerHTML = content;
}
 
function InhaltPost() {
    if (http_request.readyState == 4){
        var answer = http_request.responseText;
        document.getElementById("loading").style.display='none';
        document.getElementById("content2").style.display='block';
        if(document.getElementById("content2").innerHTML != answer){
            document.getElementById("content2").innerHTML = answer;
        }
    }
}

window.onload = "load_data()";
interval = window.setInterval("load_data();", 20000);

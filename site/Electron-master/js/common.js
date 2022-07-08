if (localStorage.getItem('status') !== undefined) {
	setNewStatus();
}

var card = document.createElement('div');
card.className = 'card';
card.id = 'statusCard'

var cardBody = document.createElement('div');
cardBody.className = 'card-body';


var button = document.createElement('button');
button.className = 'btn btn-primary';
button.id = 'saveButton'
button.innerHtml = button.innerHtml + 'Сохранить';

window.fix = null;

var inputAdd = document.createElement('input');
inputAdd.type = 'text';
inputAdd.id = 'statusInput'

var elem = document.getElementById("status");
elem.onclick = function(){
  this.parentElement.appendChild(card);
  card.appendChild(cardBody);
  cardBody.appendChild(inputAdd);
  document.getElementById('statusInput').value = 'Введите статус';
  cardBody.appendChild(button);
  button.appendChild(document.createTextNode('Сохранить'));
  window.fix = 1;
}

var statusOpen = setInterval(function(){
	if (window.fix) {
		var elem2 = document.getElementById('saveButton');
		elem2.onclick = function(){
			var newStatus = document.getElementById('statusInput').value;
			localStorage.setItem('status', newStatus);
			setNewStatus();
			document.getElementById('statusCard').remove();
		}
	}
}, 1000)

function setNewStatus(){
	document.getElementById('statusSpan').remove();
	var newSpan = document.createElement('span');
	newSpan.id = 'statusSpan';
	document.getElementById('status').appendChild(newSpan);
	document.getElementById('statusSpan').appendChild(document.createTextNode(localStorage.getItem('status')))
}
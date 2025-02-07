export default {
	myVar1: [],
	myVar2: {},
	
	// Função para verificar o comprimento do texto em Input1
	checkInputLength () {
		// Verifica se o comprimento do texto em Input1 é maior que 0
		if (Input1.text.length > 0 && Input1Copy.text.length > 0 ) {
			Button1.setVisibility(true); 
			Button1.setDisabled(false);
			 // Habilita o botão se o comprimento for maior que 0
		} else {
			Button1.setVisibility(false); 
			Button1.setDisabled(true);
			//Button1.setDisabled(true);   // Desabilita o botão se o comprimento for 0 ou menor
		}
	}
}

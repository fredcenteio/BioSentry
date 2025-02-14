export default {
  myVar1: [],
  myVar2: {},

  // Função para verificar o comprimento do texto em todas as entradas
  checkInputLength() {
    const inputCodigo = InputCodigo.text;

    if (
      inputCodigo && inputCodigo.length > 0

    ) {
      Button2.setVisibility(true); 
      Button2.setDisabled(false);
    } else {
      Button2.setVisibility(false); 
      Button2.setDisabled(true);
    }
  }
  }
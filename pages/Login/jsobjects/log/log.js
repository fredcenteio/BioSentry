export default {
  async handleLogin() {
   if (Query1.data.length > 0) {
   storeValue('user', Query1.data[0]);  // Armazena o usuário logado
  showAlert('Login bem-sucedido!', 'success');
		 
  navigateTo('Home');  // Redireciona para a página principal
	} else {
  showAlert('Credenciais inválidas', 'error');
		
	}

  }
	
	
};

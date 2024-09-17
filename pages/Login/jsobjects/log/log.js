export default {
  async handleLogin() {
    const email = Input1.text;
    const password = Input2.text;

    const response = await loginApi.run({
      email: email,
      password: password
    });

    if (response.success) {
      showAlert('Login bem-sucedido!', 'success');
      navigateTo('PáginaPrincipal');
    } else {
      showAlert('Email ou senha incorretos.', 'error');
    }
  }
};

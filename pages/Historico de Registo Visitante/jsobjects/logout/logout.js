export default {
  handleLogout() {
    storeValue('user', null);
    navigateTo('Login');
  },
  startLogoutTimer() {
    // Define o tempo em milissegundos (por exemplo, 5 minutos)
    const logoutTime = 5 * 60 * 1000;

    setTimeout(() => {
      this.handleLogout();
    }, logoutTime);
  },

};

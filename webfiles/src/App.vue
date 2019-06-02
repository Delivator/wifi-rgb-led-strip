<template>
  <v-app :dark="darkMode">
    <v-content>
      <router-view :darkMode="darkMode"/>
    </v-content>
  </v-app>
</template>

<script>
export default {
  name: "App",
  data: () => ({
    darkMode: true
  })
};
</script>

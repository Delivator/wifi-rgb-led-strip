<template>
  <v-container fluid fill-height>
    <v-layout align-center justify-center>
      <v-flex xs12 sm8 md4>
        <v-card class="elevation-12">
          <v-toolbar color="primary">
            <v-toolbar-title>Color</v-toolbar-title>
          </v-toolbar>
          <v-card-text>
            <v-layout align-center justify-center>
              <v-flex xs12>
                <color-picker v-bind="color" @input="onInput" @change="onChange" variant="persistent"></color-picker>
              </v-flex>
              <v-flex xs12>
                <v-slider v-model="color.luminosity" step="1" min="0" max="50" label="Bightness"></v-slider>
              </v-flex>
            </v-layout>
          </v-card-text>
        </v-card>
      </v-flex>
    </v-layout>
  </v-container>
</template>

<script>
const ColorPicker = () => import("@radial-color-picker/vue-color-picker");

export default {
  props: ["darkMode"],
  components: {
    ColorPicker
  },
  data: () => ({
    color: {
      hue: 50,
      saturation: 100,
      luminosity: 50,
      alpha: 1
    }
  }),
  methods: {
    onInput(hue) {
      this.color.hue = hue;
    },
    onChange() {
      console.log("Toggle LED");
    }
  }
};
</script>

<style>
@import '~@radial-color-picker/vue-color-picker/dist/vue-color-picker.min.css';
</style>
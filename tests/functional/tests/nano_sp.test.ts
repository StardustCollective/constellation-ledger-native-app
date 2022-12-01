import Zemu, { DEFAULT_START_OPTIONS } from "@zondax/zemu";
const LCTI = require("ledger-constellation-transport-interface");

import {
  APP_SEED,
  BIP_PATH,
  EXPECTED_TRANSACTION_SIGNATURE_SP,
  EXPECTED_MESSAGE_SIGNATURE,
  TX_HEX_DATA_BUFFER_1,
  TX_HEX_DATA_BUFFER_2,
  MSG_HEX_DATA_BUFFER_1,
  models,
} from "./common";

const defaultOptions = {
  ...DEFAULT_START_OPTIONS,
  logging: true,
  custom: `-s "${APP_SEED}"`,
  X11: false,
};

jest.setTimeout(120000);

beforeAll(async () => {
  await Zemu.checkAndPullImage();
});

describe("Nano S Plus", function () {
  test("Should return the correct transaction signature", async function () {
    const sim = new Zemu(models.nano_sp.path);
    try {
      await sim.start({ ...defaultOptions, model: models.nano_sp.name });
      const transport = sim.getTransport();

      await transport.send(0x80, 0x02, 0x00, 0x00, TX_HEX_DATA_BUFFER_1, [
        0x9000,
      ]);
      transport
        .send(0x80, 0x02, 0x80, 0x00, TX_HEX_DATA_BUFFER_2, [0x9000])
        .then((buffer) => {
          const signature = buffer.toString("hex");
          expect(signature).toEqual(EXPECTED_TRANSACTION_SIGNATURE_SP);
        });

      await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());

      // Sign transaction
      await sim.clickRight();
      await sim.clickRight();
      await sim.clickRight();
      await sim.clickRight();
      await sim.clickRight();
      await sim.clickBoth();
    } finally {
      await sim.close();
    }
  });
  test("Should return the correct message signature", async function () {
    const sim = new Zemu(models.nano_sp.path);
    try {
      await sim.start({ ...defaultOptions, model: models.nano_sp.name });
      const transport = sim.getTransport();

      // Enable Blind Signing
      await sim.clickRight();
      await sim.clickRight();
      await sim.clickBoth();
      await sim.clickBoth();
      await sim.clickRight();
      await sim.clickBoth();

      transport
        .send(0x80, 0x06, 0x80, 0x00, MSG_HEX_DATA_BUFFER_1, [0x9000])
        .then((buffer) => {
          const signature = buffer.toString("hex");
          expect(signature).toEqual(EXPECTED_MESSAGE_SIGNATURE);
        });

      await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());

      // Sign Message
      await sim.clickRight();
      await sim.clickRight();
      await sim.clickBoth();
    } finally {
      await sim.close();
    }
  });
  test("Should return the correct public key for Nano X", async function () {
    const sim = new Zemu(models.nano_sp.path);
    try {
      const expectedPublicKey =
        "04598f922b6786d82121b11ab74fe9b59edc5e5606df477d0158dd75934e94710b375cb87166b68ae6b451dff858ffedfee4af6afc2dd8075974c912f98e5cc0a39000";
      await sim.start({ ...defaultOptions, model: models.nano_sp.name });
      const transport = sim.getTransport();
      // Message Buffer
      const messageBuffer = Buffer.from(BIP_PATH);
      // Get Public Key
      const buffer = await transport.send(
        0x80,
        0x04,
        0x00,
        0x00,
        messageBuffer,
        [0x9000]
      );
      const publicKey = buffer.toString("hex");

      expect(publicKey).toEqual(expectedPublicKey);
    } finally {
      await sim.close();
    }
  });
  test("should display Constellation Application Ready screen correctly", async function () {
    const sim = new Zemu(models.nano_sp.path);
    try {
      await sim.start({ ...defaultOptions, model: models.nano_sp.name });
      await sim.navigateAndCompareSnapshots(
        ".",
        `${models.nano_sp.prefix.toLowerCase()}-Application-Ready-Screen`,
        []
      );
    } finally {
      await sim.close();
    }
  });
  test("should display Version screen correctly", async function () {
    const sim = new Zemu(models.nano_sp.path);
    try {
      await sim.start({ ...defaultOptions, model: models.nano_sp.name });
      await sim.clickRight();
      await sim.navigateAndCompareSnapshots(
        ".",
        `${models.nano_sp.prefix.toLowerCase()}-Version-Screen`,
        []
      );
    } finally {
      await sim.close();
    }
  });
  test(
    "should display settings screen correctly",
    async function () {
      const sim = new Zemu(models.nano_sp.path);
      try {
        await sim.start({ ...defaultOptions, model: models.nano_sp.name });
        await sim.clickRight();
        await sim.clickRight();

        await sim.navigateAndCompareSnapshots(
          ".",
          `${models.nano_sp.prefix.toLowerCase()}-Settings-Screen`,
          []
        );
      } finally {
        await sim.close();
      }
    }
  );
  test(
    "should display blind signing disabled screen correctly",
    async function () {
      const sim = new Zemu(models.nano_sp.path);
      try {
        await sim.start({ ...defaultOptions, model: models.nano_sp.name });
        await sim.clickRight();
        await sim.clickRight();
        await sim.clickBoth();
        await sim.navigateAndCompareSnapshots(
          ".",
          `${models.nano_sp.prefix.toLowerCase()}-Blind-Signing-Disabled`,
          []
        );
      } finally {
        await sim.close();
      }
    }
  );
  test(
    "should display blind signing enabled screen correctly",
    async function () {
      const sim = new Zemu(models.nano_sp.path);
      try {
        await sim.start({ ...defaultOptions, model: models.nano_sp.name });
        await sim.clickRight();
        await sim.clickRight();
        await sim.clickBoth();
        await sim.clickBoth();
        await sim.navigateAndCompareSnapshots(
          ".",
          `${models.nano_sp.prefix.toLowerCase()}-Blind-Signing-Enabled`,
          []
        );
      } finally {
        await sim.close();
      }
    }
  );
  test("Should display transaction signing flow review transactions screen correctly ", async function () {
    const sim = new Zemu(models.nano_sp.path);
    try {
      
      await sim.start({ ...defaultOptions, model: models.nano_sp.name });
      const transport = sim.getTransport();

      await transport.send(0x80, 0x02, 0x00, 0x00, TX_HEX_DATA_BUFFER_1, [
        0x9000,
      ]);
      transport.send(0x80, 0x02, 0x80, 0x00, TX_HEX_DATA_BUFFER_2, [0x9000])
    
      await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());

      Zemu.delay(1000);

      await sim.navigateAndCompareSnapshots(
        ".",
        `${models.nano_sp.prefix.toLowerCase()}-Review-transaction`,
        []
      );

      await sim.clickRight();
      await sim.clickRight();
      await sim.clickRight();
      await sim.clickRight();
      await sim.clickRight();
      await sim.clickBoth();

    } finally {
      await sim.close();
    }
  });
  test("Should display transaction signing flow from address screen correctly ", async function () {
    const sim = new Zemu(models.nano_sp.path);
    try {
      
      await sim.start({ ...defaultOptions, model: models.nano_sp.name });
      const transport = sim.getTransport();
      await transport.send(0x80, 0x02, 0x00, 0x00, TX_HEX_DATA_BUFFER_1, [
        0x9000,
      ]);
      transport.send(0x80, 0x02, 0x80, 0x00, TX_HEX_DATA_BUFFER_2, [0x9000])
    
      await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
      await sim.clickRight();
      await sim.navigateAndCompareSnapshots(
        ".",
        `${models.nano_sp.prefix.toLowerCase()}-From-Address`,
        []
      );
      await sim.clickRight();
      await sim.clickRight();
      await sim.clickRight();
      await sim.clickRight();
      await sim.clickBoth();

    } finally {
      await sim.close();
    }
  });
  test("Should display transaction signing flow to address screen correctly ", async function () {
    const sim = new Zemu(models.nano_sp.path);
    try {
      
      await sim.start({ ...defaultOptions, model: models.nano_sp.name });
      const transport = sim.getTransport();
      await transport.send(0x80, 0x02, 0x00, 0x00, TX_HEX_DATA_BUFFER_1, [
        0x9000,
      ]);
      transport.send(0x80, 0x02, 0x80, 0x00, TX_HEX_DATA_BUFFER_2, [0x9000])
    
      await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
      await sim.clickRight();
      await sim.clickRight();
      await sim.navigateAndCompareSnapshots(
        ".",
        `${models.nano_sp.prefix.toLowerCase()}-To-Address`,
        []
      );
      await sim.clickRight();
      await sim.clickRight();
      await sim.clickRight();
      await sim.clickBoth();

    } finally {
      await sim.close();
    }
  });
  test("Should display transaction signing flow amount screen correctly ", async function () {
    const sim = new Zemu(models.nano_sp.path);
    try {
      
      await sim.start({ ...defaultOptions, model: models.nano_sp.name });
      const transport = sim.getTransport();
      await transport.send(0x80, 0x02, 0x00, 0x00, TX_HEX_DATA_BUFFER_1, [
        0x9000,
      ]);
      transport.send(0x80, 0x02, 0x80, 0x00, TX_HEX_DATA_BUFFER_2, [0x9000])
    
      await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
      await sim.clickRight();
      await sim.clickRight();
      await sim.clickRight();
      await sim.navigateAndCompareSnapshots(
        ".",
        `${models.nano_sp.prefix.toLowerCase()}-Amount`,
        []
      );
      await sim.clickRight();
      await sim.clickRight();
      await sim.clickBoth();

    } finally {
      await sim.close();
    }
  });
  test("Should display transaction signing flow fee screen correctly ", async function () {
    const sim = new Zemu(models.nano_sp.path);
    try {
      
      await sim.start({ ...defaultOptions, model: models.nano_sp.name });
      const transport = sim.getTransport();
      await transport.send(0x80, 0x02, 0x00, 0x00, TX_HEX_DATA_BUFFER_1, [
        0x9000,
      ]);
      transport.send(0x80, 0x02, 0x80, 0x00, TX_HEX_DATA_BUFFER_2, [0x9000])
    
      await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
      await sim.clickRight();
      await sim.clickRight();
      await sim.clickRight();
      await sim.clickRight();
      await sim.navigateAndCompareSnapshots(
        ".",
        `${models.nano_sp.prefix.toLowerCase()}-Fee`,
        []
      );
      await sim.clickRight();
      await sim.clickBoth();

    } finally {
      await sim.close();
    }
  });
  test("Should display transaction signing flow sign transaction screen correctly ", async function () {
    const sim = new Zemu(models.nano_sp.path);
    try {
      
      await sim.start({ ...defaultOptions, model: models.nano_sp.name });
      const transport = sim.getTransport();
      await transport.send(0x80, 0x02, 0x00, 0x00, TX_HEX_DATA_BUFFER_1, [
        0x9000,
      ]);
      transport.send(0x80, 0x02, 0x80, 0x00, TX_HEX_DATA_BUFFER_2, [0x9000])
    
      await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
      await sim.clickRight();
      await sim.clickRight();
      await sim.clickRight();
      await sim.clickRight();
      await sim.clickRight();
      await sim.navigateAndCompareSnapshots(
        ".",
        `${models.nano_sp.prefix.toLowerCase()}-Sign-Transaction`,
        []
      );
      await sim.clickBoth();

    } finally {
      await sim.close();
    }
  });
  test("Should display transaction signing flow Deny transaction screen correctly", async function () {
    const sim = new Zemu(models.nano_sp.path);
    try {
      
      await sim.start({ ...defaultOptions, model: models.nano_sp.name });
      const transport = sim.getTransport();
      await transport.send(0x80, 0x02, 0x00, 0x00, TX_HEX_DATA_BUFFER_1, [
        0x9000,
      ]);
      transport.send(0x80, 0x02, 0x80, 0x00, TX_HEX_DATA_BUFFER_2, [0x9000])
    
      await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
      await sim.clickRight();
      await sim.clickRight();
      await sim.clickRight();
      await sim.clickRight();
      await sim.clickRight();
      await sim.clickRight();
      await sim.navigateAndCompareSnapshots(
        ".",
        `${models.nano_sp.prefix.toLowerCase()}-Deny-Transaction`,
        []
      );
      await sim.clickLeft();
      await sim.clickBoth();

    } finally {
      await sim.close();
    }
  });
  test("Should display message signing flow Review Message screen correctly", async function () {
    const sim = new Zemu(models.nano_sp.path);
    try {
      await sim.start({ ...defaultOptions, model: models.nano_sp.name });
      const transport = sim.getTransport();

      // Enable Blind Signing
      await sim.clickRight();
      await sim.clickRight();
      await sim.clickBoth();
      await sim.clickBoth();
      await sim.clickRight();
      await sim.clickBoth();

      transport
        .send(0x80, 0x06, 0x80, 0x00, MSG_HEX_DATA_BUFFER_1, [0x9000])
        .then((buffer) => {
          const signature = buffer.toString("hex");
          expect(signature).toEqual(EXPECTED_MESSAGE_SIGNATURE);
        });

      await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());

      await sim.navigateAndCompareSnapshots(
        ".",
        `${models.nano_sp.prefix.toLowerCase()}-Review-Message`,
        []
      );

      await sim.clickRight();
      await sim.clickRight();
      await sim.clickBoth();

    } finally {
      await sim.close();
    }
  });
  test("Should display message signing flow Warning Blind Signing screen correctly", async function () {
    const sim = new Zemu(models.nano_sp.path);
    try {
      await sim.start({ ...defaultOptions, model: models.nano_sp.name });
      const transport = sim.getTransport();

      // Enable Blind Signing
      await sim.clickRight();
      await sim.clickRight();
      await sim.clickBoth();
      await sim.clickBoth();
      await sim.clickRight();
      await sim.clickBoth();

      transport
        .send(0x80, 0x06, 0x80, 0x00, MSG_HEX_DATA_BUFFER_1, [0x9000])
        .then((buffer) => {
          const signature = buffer.toString("hex");
          expect(signature).toEqual(EXPECTED_MESSAGE_SIGNATURE);
        });

      await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
      
      await sim.clickRight();

      await sim.navigateAndCompareSnapshots(
        ".",
        `${models.nano_sp.prefix.toLowerCase()}-Warning-Blind-Signing`,
        []
      );

      await sim.clickRight();
      await sim.clickBoth();

    } finally {
      await sim.close();
    }
  });

  test("Should display message signing flow Sign Message screen correctly", async function () {
    const sim = new Zemu(models.nano_sp.path);
    try {
      await sim.start({ ...defaultOptions, model: models.nano_sp.name });
      const transport = sim.getTransport();

      // Enable Blind Signing
      await sim.clickRight();
      await sim.clickRight();
      await sim.clickBoth();
      await sim.clickBoth();
      await sim.clickRight();
      await sim.clickBoth();

      transport
        .send(0x80, 0x06, 0x80, 0x00, MSG_HEX_DATA_BUFFER_1, [0x9000])
        .then((buffer) => {
          const signature = buffer.toString("hex");
          expect(signature).toEqual(EXPECTED_MESSAGE_SIGNATURE);
        });

      await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
      
      await sim.clickRight();
      await sim.clickRight();

      await sim.navigateAndCompareSnapshots(
        ".",
        `${models.nano_sp.prefix.toLowerCase()}-Sign-Message`,
        []
      );

      await sim.clickBoth();

    } finally {
      await sim.close();
    }
  });
});

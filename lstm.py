# %%
from keras.models import Sequential
from keras.layers import Dense
from keras.layers import LSTM
from keras.layers import TimeDistributed
import tensorflow
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
plt.rc('font', size=15)

def series_to_supervised(data, n_in=1, n_out=1, dropnan=True):
    # Get the number of variables in the dataset
    if type(data) == list:
        n_vars = 1
    else:
        n_vars = data.shape[1]
    #n_vars = 1 if type(data) is list else data.shape[1]
    # Convert the data to a dataframe if it is not.
    df = pd.DataFrame(data)
    # Initialize temporary lists that will append the shifted columns
    columns, var_name = list(), list()
    # Append columns with the number of input sequence and shift the data everytime a column is created
    for i in range(n_in, 0, -1):
        columns.append(df.shift(i))
        var_name += [('var%d(t-%d)' % (j+1, i)) for j in range(n_vars)]
    # Append columns for present and future forecast with the inverse of the shift function
    for i in range(0, n_out):
        columns.append(df.shift(-i))
        if i == 0:
            var_name += [('var%d(t)' % (j+1)) for j in range(n_vars)]
        else:
            var_name += [('var%d(t+%d)' % (j+1, i)) for j in range(n_vars)]
    # Concatenate all the columns
    agg = pd.concat(columns, axis=1)
    agg.columns = var_name
    # drop rows with missing values
    if dropnan:
        agg.dropna(inplace=True)
    return agg

def create_dummies(data, column_name, prefix=None):

    return pd.concat([data,pd.get_dummies(data[column_name], prefix=prefix)],axis=1).drop(column_name, axis=1)

def train_lstm(data, n_id, n_output, lag, neuron1, neuron2, batch_size, epochs):
    # Create a copy of the dataset
    df = data.copy()
    #n_output = 1
    # Create supervised problem
    df_sup = series_to_supervised(df,lag,1)
    # drop the features we do not want do predict
    df_sup.drop(df_sup.columns[(-7-n_output-n_id):-n_output], axis=1, inplace=True)
    # get the number of variables without the output values
    n_var = len(df.columns.values)-n_output
    # split into input and outputs
    X_train, y_train = df_sup.iloc[:, :lag*n_var].values, df_sup.iloc[:, -n_output:].values
    # reshape input to be [samples, timesteps, features]
    X_train = X_train.reshape((X_train.shape[0], lag, n_var))
    # design the neural network
    model = Sequential()
    model.add(LSTM(neuron1, activation='tanh', input_shape=(X_train.shape[1], X_train.shape[2])))
    model.add(Dense(n_output, activation='softmax'))
    model.compile(loss='categorical_crossentropy', optimizer='adam', metrics=['accuracy'])
    # fit network
    model_fit = model.fit(X_train, y_train, epochs=epochs,batch_size=batch_size, shuffle=False)
    # plot history
    plt.plot(model_fit.history['accuracy'], label='Train')
    plt.legend()
    plt.show()

    return model, X_train
# %%
df = pd.read_csv('traindata_.csv', dtype={'ID': str})
# %%
n_id = len(df.ID.unique())
n_output = len(df.NextHeuristic.unique())
df = create_dummies(df, 'ID', prefix='ID')
#move_column = df.pop('NextHeuristic')
#df.insert(7+n_id,'NextHeuristic',move_column)
df = create_dummies(df, 'NextHeuristic')
# %%
tensorflow.keras.backend.clear_session()
model, X_train = train_lstm(data=df, n_id=n_id, n_output=n_output, lag=2, neuron1=50, neuron2=30, batch_size=64, epochs=100)
# %%
test = np.array([0.6151315789473685,0.75,0.32334037150482053,0.514391447368421,0.484375,0.3145416770940778,0.5226817436913052,1])
test = np.pad(test, (0,n_id-1),'constant')
X_test = test.reshape((1,1,(7+n_id)))
yhat = model.predict(X_test)
yhat
# %%

